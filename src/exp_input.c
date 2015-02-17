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

#define BUFFER_SIZE 4096

typedef struct {
     input_t fn;
     logger_t log;
} input_impl_t;

line_t *new_line(line_t *previous, size_t length, char *content) {
     line_t *result = malloc(sizeof(line_t) + length + 1);
     if (previous != NULL) {
          previous->next = result;
     }
     result->length = length;
     memcpy(result->buffer, content, length + 1);
     return result;
}

line_t *read_all_lines(input_impl_t *this, FILE *in) {
     static char buffer[BUFFER_SIZE];
     static char line[BUFFER_SIZE];

     line_t *result = NULL;
     line_t *last = NULL;

     size_t buffer_length;
     size_t line_length = 0;
     size_t buffer_index;
     bool_t line_too_long_flag = false;
     size_t count_lines_too_long;

     while ((buffer_length = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
          for (buffer_index = 0; buffer_index < buffer_length; buffer_index++) {
               if (buffer[buffer_index] == '\n') {
                    last = new_line(last, line_length, line);
                    if (result == NULL) {
                         result = last;
                    }
                    line_length = 0;
                    line_too_long_flag = false;
               } else if (line_length >= BUFFER_SIZE) {
                    if (!line_too_long_flag) {
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
     }

     if (count_lines_too_long > 0) {
          this->log(warn, "%lu line%s too long, truncated to %d characters\n", (unsigned long)count_lines_too_long, count_lines_too_long > 1 ? "s" : "", BUFFER_SIZE - 1);
     }

     return result;
}

static void do_parse(input_impl_t *this, FILE *in) {
     line_t *lines = read_all_lines(this, in);
}

static void impl_parse(input_impl_t *this, char *filename) {
     FILE *in;
     if (!strcmp("-", filename)) {
          in = stdin;
          this->log(debug, "Using stdin");
     } else {
          in = fopen(filename, "r");
          if (!in) {
               this->log(warn, "Unknown file: %s", filename);
               return;
          }
          this->log(debug, "Opening file: %s", filename);
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

#include "exp_input.h"
