/*
  This file is part of exp.

  exp is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 3 of the License.

  exp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with exp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "exp_input.h"
#include "exp_entry.h"
#include "exp_entry_factory.h"

#define BUFFER_SIZE 4096
#define SAMPLE_SIZE 10
#define TALLY_THRESHOLD (SAMPLE_SIZE / 4)

typedef struct {
     input_t fn;
     logger_t log;
     size_t length;
     entry_t **entries;
} input_impl_t;

static line_t *new_line(line_t *previous, size_t length, char *content) {
     line_t *result = malloc(sizeof(line_t) + length + 1);
     if (previous != NULL) {
          previous->next = result;
     }
     result->length = length;
     memcpy(result->buffer, content, length + 1);
     return result;
}

static line_t *read_all_lines(input_impl_t *this, FILE *in) {
     static char buffer[BUFFER_SIZE];
     static char line[BUFFER_SIZE];

     line_t *result = NULL;
     line_t *last = NULL;

     size_t buffer_length;
     size_t line_length = 0;
     size_t buffer_index;
     bool_t line_too_long_flag = false;
     size_t count_lines_too_long = 0;

     this->length = 0;

     while ((buffer_length = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
          for (buffer_index = 0; buffer_index < buffer_length; buffer_index++) {
               if (buffer[buffer_index] == '\n') {
                    last = new_line(last, line_length, line);
                    if (result == NULL) {
                         result = last;
                    }
                    line_length = 0;
                    line_too_long_flag = false;
                    this->length++;
               } else if (line_length >= BUFFER_SIZE) {
                    if (!line_too_long_flag) {
                         this->log(info, "Truncating line %lu\n", (unsigned long)this->length);
                         count_lines_too_long++;
                         line_too_long_flag = true;
                         line[BUFFER_SIZE - 1] = '\0';
                    }
               } else {
                    line[line_length++] = buffer[buffer_index];
               }
          }
     }

     if (ferror(in)) {
          this->log(warn, "Error during parse: %s\n", strerror(errno));
          while(result != NULL) {
               last = result->next;
               free(result);
               result = last;
          }
     } else if (line_length > 0) {
          last = new_line(last, line_length, line);
          if (result == NULL) {
               result = last;
          }
          this->length++;
     }

     if (count_lines_too_long > 0) {
          this->log(warn, "%lu line%s too long, truncated to %d characters\n", (unsigned long)count_lines_too_long, count_lines_too_long > 1 ? "s" : "", BUFFER_SIZE - 1);
     }
     this->log(debug, "Read %lu line%s\n", (unsigned long)this->length, this->length > 1 ? "s" : "");

     return result;
}

static entry_factory_t *select_entry_factory(input_impl_t *this, line_t *lines) {
     entry_factory_t *result = NULL;
     int i, f;
     int nf = entry_factories_length();
     line_t *line = lines;
     entry_factory_t *factory;
     size_t *tally = malloc(sizeof(size_t) * nf);
     memset(tally, 0, sizeof(size_t) * nf);

     for (i = 0; line != NULL && i < SAMPLE_SIZE; i++, line = line->next) {
          for (f = 0; f < nf; f++) {
               factory = entry_factory(f);
               if (factory->is_type(factory, line)) {
                    tally[f]++;
               }
          }
     }

     for (f = 0; f < nf; f++) {
          factory = entry_factory(f);
          this->log(debug, "tally[%s] = %lu\n", factory->get_name(factory), (unsigned long)tally[f]);
     }

     for (f = 0; f < nf; f++) {
          factory = entry_factory(f);
          if (factory->tally_logic(factory, tally[f], TALLY_THRESHOLD, SAMPLE_SIZE)) {
               result = factory;
          }
     }

     return result;
}

static void do_parse(input_impl_t *this, FILE *in) {
     line_t *lines = read_all_lines(this, in);
     entry_factory_t *factory = select_entry_factory(this, lines);
     line_t *line;
     int i = 0;

     if (factory == NULL) {
        this->log(warn, "Factory not found!!\n");
     } else {
        this->entries = malloc(this->length * sizeof(entry_t*));
        for (line = lines; line != NULL; line = line->next) {
           this->entries[i++] = factory->new_entry(factory, line);
        }
     }
}

static void impl_parse(input_impl_t *this, char *filename) {
     FILE *in;
     if (!strcmp("-", filename)) {
          in = stdin;
          this->log(debug, "Using stdin\n");
     } else {
          in = fopen(filename, "r");
          if (!in) {
               this->log(warn, "%s: %s\n", strerror(errno), filename);
               return;
          }
          this->log(debug, "Opening file: %s\n", filename);
     }

     do_parse(this, in);

     if (in != stdin) {
          fclose(in);
     }
}

input_t input_impl_fn = {
     .parse = (input_parse_fn)impl_parse,
};

input_t *new_input(logger_t log) {
     input_impl_t *result = malloc(sizeof(input_impl_t));
     result->fn = input_impl_fn;
     result->log = log;
     return &(result->fn);
}
