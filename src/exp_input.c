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

typedef struct {
     input_t fn;
     logger_t log;
} input_impl_t;

static void impl_parse(input_impl_t *this, char *filename) {
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
