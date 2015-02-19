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

#include "exp_output.h"

typedef struct {
     output_t fn;
} output_wordcount_t;

static void output_wordcount_display(output_wordcount_t *this) {
}

static options_set_t output_wordcount_options_set(output_wordcount_t *this) {
     static options_set_t result = {
          false, false, false, false, false,
     };
     return result;
}

static output_t output_wordcount_fn = {
     .fingerprint_file = NULL,
     .options_set = (output_options_set_fn)output_wordcount_options_set,
     .display = (output_display_fn)output_wordcount_display,
};

output_t *new_output_wordcount(logger_t log, input_t *input, output_options_t options) {
     output_wordcount_t *result = malloc(sizeof(output_wordcount_t));
     result->fn = output_wordcount_fn;
     return &(result->fn);
}
