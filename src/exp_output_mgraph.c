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
} output_mgraph_t;

static void output_mgraph_display(output_mgraph_t *this) {
}

static output_t output_mgraph_fn = {
   .display = (output_display_fn)output_mgraph_display,
};

output_t *new_output_mgraph(logger_t log, input_t *input, output_options_t options) {
   output_mgraph_t *result = malloc(sizeof(output_mgraph_t));
   result->fn = output_mgraph_fn;
   return &(result->fn);
}
