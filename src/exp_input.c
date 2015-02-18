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
     input_file_t fn;
     logger_t log;
     file_t *file;
     entry_factory_t *factory;
     size_t length;
     entry_t **entries;
     char filename[0];
} input_file_impl_t;

static entry_factory_t *impl_get_factory(input_file_impl_t *this) {
     return this->factory;
}

static const char *impl_get_name(input_file_impl_t *this) {
     return this->filename;
}

static size_t impl_entries_length(input_file_impl_t *this) {
     return this->length;
}

static entry_t *impl_entry(input_file_impl_t *this, int index) {
     return this->entries[index];
}

static input_file_t input_file_impl_fn = {
     .get_factory = (input_file_get_factory_fn)impl_get_factory,
     .get_name = (input_file_get_name_fn)impl_get_name,
     .entries_length = (input_file_entries_length_fn)impl_entries_length,
     .entry = (input_file_entry_fn)impl_entry,
};

typedef struct {
     input_t fn;
     logger_t log;
     size_t count;
     size_t capacity;
     input_file_impl_t **files;
} input_impl_t;

static size_t impl_files_length(input_impl_t *this) {
     return this->count;
}

static input_file_t *impl_file(input_impl_t *this, int index) {
     return &(this->files[index]->fn);
}

static entry_factory_t *select_entry_factory(logger_t log, line_t *lines) {
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
          log(debug, "tally[%s] = %lu\n", factory->get_name(factory), (unsigned long)tally[f]);
     }

     for (f = 0; f < nf; f++) {
          factory = entry_factory(f);
          if (factory->tally_logic(factory, tally[f], TALLY_THRESHOLD, SAMPLE_SIZE)) {
               result = factory;
          }
     }

     return result;
}

static input_file_impl_t *do_parse(input_impl_t *this, file_t *in, const char *filename) {
     input_file_impl_t *result = NULL;
     line_t *lines = in->lines(in);
     entry_factory_t *factory = select_entry_factory(this->log, lines);
     line_t *line;
     int i = 0;

     if (factory == NULL) {
          this->log(warn, "Input factory not found for file %s\n", filename);
     } else {
          result = malloc(sizeof(input_file_impl_t) + strlen(filename + 1));
          result->fn = input_file_impl_fn;
          result->log = this->log;
          result->file = in;
          result->factory = factory;
          result->length = in->lines_count(in);
          result->entries = malloc(result->length * sizeof(entry_t*));
          strcpy(result->filename, filename);
          for (line = lines; line != NULL; line = line->next) {
               result->entries[i++] = factory->new_entry(factory, line);
          }
     }

     return result;
}

static input_file_impl_t *impl_parse(input_impl_t *this, const char *filename) {
     input_file_impl_t *result = NULL;
     file_t *in = new_file(this->log, warn, filename);
     if (in != NULL) {
          result = do_parse(this, in, filename);
          if (result != NULL) {
               if (this->count == this->capacity) {
                    if (this->capacity == 0) {
                         this->capacity = 4;
                         this->files = malloc(this->capacity * sizeof(input_file_t*));
                    } else {
                         this->capacity *= 2;
                         this->files = realloc(this->files, this->capacity * sizeof(input_file_t*));
                    }
               }
               this->files[this->count++] = result;
          }
     }
     return result;
}

static input_t input_impl_fn = {
     .parse = (input_parse_fn)impl_parse,
     .files_length = (input_files_length_fn)impl_files_length,
     .file = (input_file_fn)impl_file,
};

input_t *new_input(logger_t log) {
     input_impl_t *result = malloc(sizeof(input_impl_t));
     result->fn = input_impl_fn;
     result->log = log;
     result->count = result->capacity = 0;
     result->files = NULL;
     return &(result->fn);
}
