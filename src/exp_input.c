/*
  This file is part of ExP.

  ExP is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, version 3 of the License.

  ExP is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ExP.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @ingroup exp_input
 * @file
 *
 * This file contains the implementation of inputs.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <cad_array.h>

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
     size_t size;
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

static size_t impl_size(input_file_impl_t *this) {
     return this->size;
}

static entry_t *impl_entry(input_file_impl_t *this, int index) {
     return this->entries[index];
}

static input_file_t input_file_impl_fn = {
     .get_factory = (input_file_get_factory_fn)impl_get_factory,
     .get_name = (input_file_get_name_fn)impl_get_name,
     .entries_length = (input_file_entries_length_fn)impl_entries_length,
     .size = (input_file_size_fn)impl_size,
     .entry = (input_file_entry_fn)impl_entry,
};

typedef struct {
     input_t fn;
     logger_t log;
     options_t options;
     cad_array_t *files;
} input_impl_t;

static size_t impl_files_length(input_impl_t *this) {
     return this->files->count(this->files);
}

static input_file_t *impl_file(input_impl_t *this, int index) {
     return this->files->get(this->files, index);
}

static int impl_file_comparator(input_file_impl_t **file1, input_file_impl_t **file2) {
     int result = (*file2)->size - (*file1)->size;
     if (result == 0) {
          result = (*file2)->length - (*file1)->length;
          if (result == 0) {
               result = strcmp((*file1)->filename, (*file2)->filename);
          }
     }
     return result;
}

static void impl_sort_files(input_impl_t *this) {
     int i, n = this->files->count(this->files);
     input_file_impl_t *file;
     this->files->sort(this->files, (comparator_fn)impl_file_comparator);
     if (this->log(debug, "Sorted files:\n")) {
          for (i = 0; i < n; i++) {
               file = this->files->get(this->files, i);
               this->log(debug, " %2d: %6lu | %4lu | %s\n", i+1, (unsigned long)file->size, (unsigned long)file->length, file->filename);
          }
     }
}

static entry_factory_t *select_entry_factory(logger_t log, file_t *file) {
     entry_factory_t *result = NULL;
     int i, f, nl, nlines;
     int nf = entry_factories_length();
     line_t *line;
     entry_factory_t *factory;
     size_t *tally = malloc(sizeof(size_t) * nf);
     bool_t found;
     memset(tally, 0, sizeof(size_t) * nf);

     nlines = file->lines_count(file);
     if (nlines > 0) {
          do {
               for (i = 0; i < SAMPLE_SIZE; i++) {
                    nl = rand() % nlines;
                    line = file->line(file, nl);
                    log(debug, "Sample line %4d/%4d [%d %s] | %.*s\n", nl+1, nlines, nf, nf == 1 ? "factory" : "factories", (int)line->length, line->buffer);
                    found = false;
                    for (f = 0; !found && f < nf; f++) {
                         factory = entry_factory(f);
                         if (factory->is_type(factory, line)) {
                              log(debug, " => %s\n", factory->get_name(factory));
                              tally[f]++;
                              found = true;
                         }
                    }
                    if (!found) {
                              log(debug, " => NOT FOUND\n");
                    }
               }

               for (f = 0; f < nf; f++) {
                    factory = entry_factory(f);
                    log(debug, "tally[%s] = %lu\n", factory->get_name(factory), (unsigned long)tally[f]);
               }

               for (f = 0; result == NULL && f < nf; f++) {
                    factory = entry_factory(f);
                    if (factory->tally_logic(factory, tally[f], TALLY_THRESHOLD, SAMPLE_SIZE)) {
                         result = factory;
                    }
               }
          } while (result == NULL);
     }

     return result;
}

static input_file_impl_t *do_parse(input_impl_t *this, file_t *in, const char *filename) {
     input_file_impl_t *result = NULL;
     entry_factory_t *factory = select_entry_factory(this->log, in);
     line_t *line;
     int i, n = in->lines_count(in);

     if (factory == NULL) {
          this->log(warn, "Input factory not found for file %s\n", filename);
     } else {
          this->log(info, "Using factory \"%s\" for file %s\n", factory->get_name(factory), filename);
          result = malloc(sizeof(input_file_impl_t) + strlen(filename) + 1);
          result->fn = input_file_impl_fn;
          result->log = this->log;
          result->file = in;
          result->factory = factory;
          result->length = in->lines_count(in);
          result->size = in->size(in);
          result->entries = malloc(result->length * sizeof(entry_t*));
          strcpy(result->filename, filename);
          for (i = 0; i < n; i++) {
               line = in->line(in, i);
               result->entries[i] = factory->new_entry(factory, line);
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
               this->files->insert(this->files, this->files->count(this->files), result);
          }
     }
     return result;
}

static options_set_t impl_options_set(input_impl_t *this) {
     static options_set_t result = {
          .filter=false,
          .fingerprint=false,
          .tick=false,
          .wide=false,
          .sample=false,
          .filter_extradirs=false,
          .fingerprint_extradirs=false,
          .factory_extradirs=true,
          .year=false,
          .exp_mode=false,
          .dev=true,
     };
     return result;
}

static options_t impl_default_options(input_impl_t *this) {
     static options_t result = {
          .factory_extradirs = NULL,
     };
     return result;
}

static void impl_set_options(input_impl_t *this, options_t options) {
     int i, n = entry_factories_length();
     entry_factory_t *factory;

     this->options = options;
     if (this->options.factory_extradirs != NULL) {
          for (i = 0; i < n; i++) {
               factory = entry_factory(i);
               factory->set_extradirs(factory, this->options.factory_extradirs);
          }
     }
}

static input_t input_impl_fn = {
     .parse = (input_parse_fn)impl_parse,
     .files_length = (input_files_length_fn)impl_files_length,
     .file = (input_file_fn)impl_file,
     .sort_files = (input_sort_files_fn)impl_sort_files,
     .options_set = (input_options_set_fn)impl_options_set,
     .default_options = (input_default_options_fn)impl_default_options,
     .set_options = (input_set_options_fn)impl_set_options,
};

input_t *new_input(logger_t log) {
     input_impl_t *result = malloc(sizeof(input_impl_t));
     result->fn = input_impl_fn;
     result->log = log;
     result->files = cad_new_array(stdlib_memory);
     return &(result->fn);
}
