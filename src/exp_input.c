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
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "exp_input.h"
#include "exp_file.h"
#include "exp_entry.h"
#include "exp_entry_factory.h"

#define SAMPLE_SIZE 10
#define TALLY_THRESHOLD (SAMPLE_SIZE / 4)

typedef struct {
     input_t fn;
     logger_t log;
     file_t *file;
     size_t length;
     entry_t **entries;
} input_impl_t;

static size_t impl_entries_length(input_impl_t *this) {
     return this->length;
}

static entry_t *impl_entry(input_impl_t *this, int index) {
     return this->entries[index];
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

static void do_parse(input_impl_t *this) {
     line_t *lines = this->file->get_lines(this->file);
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
     file_t *in = new_file(this->log, warn, filename);
     this->file = in;
     if (in != NULL) {
          do_parse(this);
     }
}

input_t input_impl_fn = {
     .parse = (input_parse_fn)impl_parse,
     .entries_length = (input_entries_length_fn)impl_entries_length,
     .entry = (input_entry_fn)impl_entry,
};

input_t *new_input(logger_t log) {
     input_impl_t *result = malloc(sizeof(input_impl_t));
     result->fn = input_impl_fn;
     result->log = log;
     return &(result->fn);
}
