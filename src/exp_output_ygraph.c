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
} output_ygraph_t;

static void output_ygraph_display(output_ygraph_t *this) {
}

static output_t output_ygraph_fn = {
   .display = (output_display_fn)output_ygraph_display,
};

output_t *new_output_ygraph(logger_t log, input_t *input, output_options_t options) {
   output_ygraph_t *result = malloc(sizeof(output_ygraph_t));
   result->fn = output_ygraph_fn;
   return &(result->fn);
}
