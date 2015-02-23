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

/**
 * @file
 * The header for inputs.
 */

#include "exp.h"
#include "exp_log.h"
#include "exp_input.h"

/**
 * @addtogroup exp_output
 * @{
 */

/**
 * The output interface.
 */
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

/**
 * The user-supplied output options
 */
typedef struct {
     bool_t   filter;
     bool_t   fingerprint;
     char    *tick;
     bool_t   wide;
     sample_t sample;
     char   **filter_extradirs;
     char   **fingerprint_extradirs;
} output_options_t;

/**
 * An options setting mask, useful to check which options were set and
 * if they were allowed to be set.
 *
 * @see output_options_set_fn
 */
typedef struct {
     bool_t filter;
     bool_t fingerprint;
     bool_t tick;
     bool_t wide;
     bool_t sample;
     bool_t filter_extradirs;
     bool_t fingerprint_extradirs;
} options_set_t;

/**
 * Called by the [fingerprint](@ref exp_fingerprint.h)
 *
 * @param[in] this the target output, that must belong to a fingerprint
 * @param[in] index the index of the fingerprint file
 * @param[in] data the output data, i.e. the output on which the fingerprint must be applied
 *
 * @return `true` if the fingerprint succeeded, `false` otherwise
 */
typedef bool_t (*output_fingerprint_file_fn)(output_t *this, int index, void *data);

/**
 * The options mask allowed by this output.
 *
 * @param[in] this the target output
 *
 * @return the options set mask
 */
typedef options_set_t (*output_options_set_fn)(output_t *this);

/**
 * Actually run the log analysis and display relevant results.
 *
 * @param[in] this the target output
 */
typedef void (*output_display_fn)(output_t *this);

struct output_s {
     /**
      * @see output_fingerprint_file_fn
      */
     output_fingerprint_file_fn fingerprint_file;
     /**
      * @see output_options_set_fn
      */
     output_options_set_fn options_set;
     /**
      * @see output_display_fn
      */
     output_display_fn display;
};

/**
 * Create a new "hash" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_hash     (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "wordcount" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_wordcount(logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "daemon" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_daemon   (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "hosy" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_host     (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "sgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_sgraph (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "mgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_mgraph (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "hgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_hgraph (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "dgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_dgraph (logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "mograph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_mograph(logger_t log, input_t *input, output_options_t options);

/**
 * Create a new "ygraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 * @param[in] options the user-supplied options
 *
 * @return the new output
 */
output_t *new_output_ygraph (logger_t log, input_t *input, output_options_t options);

/**
 * @}
 */

#endif /* __EXP_OUTPUT_H__ */
