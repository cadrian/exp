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

#ifndef __EXP_OUTPUT_H__
#define __EXP_OUTPUT_H__

#include "exp.h"
#include "exp_log.h"
#include "exp_input.h"

typedef struct output_s output_t;

/**
 * The sample type.
 */
typedef enum {
     /** don't sample */
     sample_none=0,
     /** sample under a threshold */
     sample_threshold=1,
     /** sample always */
     sample_all=2
} sample_t;

/**
 * The mode type. Defines the output mode.
 */
typedef enum {
     mode_undefined=0,
     mode_hash,
     mode_wordcount,
     mode_daemon,
     mode_host,
     mode_sgraph,
     mode_mgraph,
     mode_hgraph,
     mode_dgraph,
     mode_mograph,
     mode_ygraph
} expmode_t;

typedef struct {
     bool_t   filter;
     bool_t   fingerprint;
     char    *tick;
     bool_t   wide;
     sample_t sample;
} output_options_t;

typedef struct {
     bool_t filter;
     bool_t fingerprint;
     bool_t tick;
     bool_t wide;
     bool_t sample;
} options_set_t;

typedef bool_t (*output_fingerprint_file_fn)(output_t *this, int index, void *data);
typedef options_set_t (*output_options_set_fn)(output_t *this);
typedef void (*output_display_fn)(output_t *this);

struct output_s {
     output_fingerprint_file_fn fingerprint_file;
     output_options_set_fn options_set;
     output_display_fn display;
};

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
