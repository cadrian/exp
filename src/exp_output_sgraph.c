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
} output_sgraph_t;

static void output_sgraph_display(output_sgraph_t *this) {
}

static output_t output_sgraph_fn = {
   .display = (output_display_fn)output_sgraph_display,
};

output_t *new_output_sgraph(logger_t log, input_t *input, output_options_t options) {
   output_sgraph_t *result = malloc(sizeof(output_sgraph_t));
   result->fn = output_sgraph_fn;
   return &(result->fn);
}
