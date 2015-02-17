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

#ifndef __EXP_OUTPUT_H__
#define __EXP_OUTPUT_H__

#include "exp.h"
#include "exp_log.h"
#include "exp_input.h"

typedef struct output_s output_t;

typedef void (*output_display_fn)(output_t *this);

struct output_s {
   output_display_fn display;
};

typedef struct {
     bool_t filter;
     bool_t fingerprint;
     char  *tick;
     bool_t wide;
} output_options_t;

output_t *new_output_hash     (logger_t log, input_t *input, output_options_t options);
output_t *new_output_wordcount(logger_t log, input_t *input, output_options_t options);
output_t *new_output_daemon   (logger_t log, input_t *input, output_options_t options);
output_t *new_output_host     (logger_t log, input_t *input, output_options_t options);

output_t *new_output_sgraph (logger_t log, input_t *input, output_options_t options);
output_t *new_output_mgraph (logger_t log, input_t *input, output_options_t options);
output_t *new_output_hgraph (logger_t log, input_t *input, output_options_t options);
output_t *new_output_dgraph (logger_t log, input_t *input, output_options_t options);
output_t *new_output_mograph(logger_t log, input_t *input, output_options_t options);
output_t *new_output_ygraph (logger_t log, input_t *input, output_options_t options);

#endif /* __EXP_OUTPUT_H__ */
