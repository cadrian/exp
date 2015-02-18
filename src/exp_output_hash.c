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

#include "exp_output.h"

typedef struct {
   output_t fn;
   logger_t log;
   input_t *input;
   output_options_t options;
} output_hash_t;

static void hash_prepare(output_hash_t *this, entry_t *entry) {
     if (this->options.filter) {
          // TODO hash_filter(this, entry, "hash.stopwords");
     }
     if (this->options.fingerprint) {
          // TODO hash_fingerprint(this, entry);
     }
     // TODO
}

static void hash_display(output_hash_t *this, entry_t *entry) {
     // TODO
}

static void output_hash_display(output_hash_t *this) {
     int i, n = this->input->entries_length(this->input);
     for (i = 0; i < n; i++) {
          hash_prepare(this, this->input->entry(this->input, i));
     }
     for (i = 0; i < n; i++) {
          hash_display(this, this->input->entry(this->input, i));
     }
}

static output_t output_hash_fn = {
   .display = (output_display_fn)output_hash_display,
};

output_t *new_output_hash(logger_t log, input_t *input, output_options_t options) {
   output_hash_t *result = malloc(sizeof(output_hash_t));
   result->fn = output_hash_fn;
   result->log = log;
   result->input = input;
   result->options = options;
   return &(result->fn);
}
