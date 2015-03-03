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
#include "exp_options.h"

/**
 * @addtogroup exp_output
 * @{
 */

/**
 * The output interface.
 */
typedef struct output_s output_t;

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
 * The default options for this output.
 *
 * @param[in] this the target output
 *
 * @return the default options
 */
typedef options_t (*output_default_options_fn)(output_t *this);

/**
 * Set the user-supplied options
 *
 * @param[in] this the target output
 * @param[in] options the user-supplied options
 */
typedef void (*output_set_options_fn)(output_t *this, options_t options);

/**
 * Prepare the log analysis and display relevant results.
 *
 * @param[in] this the target output
 */
typedef void (*output_prepare_fn)(output_t *this);

/**
 * Actually run the log analysis and display relevant results. Must be prepared first.
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
      * @see output_default_options_fn
      */
     output_default_options_fn default_options;
     /**
      * @see output_set_options_fn
      */
     output_set_options_fn set_options;
     /**
      * @see output_prepare_fn
      */
     output_prepare_fn prepare;
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
 *
 * @return the new output
 */
output_t *new_output_hash(logger_t log, input_t *input);

/**
 * Create a new "wordcount" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_wordcount(logger_t log, input_t *input);

/**
 * Create a new "daemon" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_daemon(logger_t log, input_t *input);

/**
 * Create a new "hosy" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_host(logger_t log, input_t *input);

/**
 * Create a new "sgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_sgraph(logger_t log, input_t *input);

/**
 * Create a new "mgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_mgraph(logger_t log, input_t *input);

/**
 * Create a new "hgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_hgraph(logger_t log, input_t *input);

/**
 * Create a new "dgraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_dgraph(logger_t log, input_t *input);

/**
 * Create a new "mograph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_mograph(logger_t log, input_t *input);

/**
 * Create a new "ygraph" output.
 *
 * @param[in] log the logger
 * @param[in] input the input files
 *
 * @return the new output
 */
output_t *new_output_ygraph(logger_t log, input_t *input);

/**
 * Ansi color codes
 * @{
 */

#define OFF         "\033[0m"
#define BLACK       "\033[0;30m"
#define DARK_RED    "\033[0;31m"
#define DARK_GREEN  "\033[0;32m"
#define BROWN       "\033[0;33m"
#define DARK_BLUE   "\033[0;34m"
#define PURPLE      "\033[0;35m"
#define DARK_CYAN   "\033[0;36m"
#define LIGHT_GRAY  "\033[0;37m"
#define DARK_GRAY   "\033[1;30m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define YELLOW      "\033[1;33m"
#define BLUE        "\033[1;34m"
#define MAGENTA     "\033[1;35m"
#define CYAN        "\033[1;36m"
#define WHITE       "\033[1;37m"

/**
 * @}
 */

/**
 * @}
 */

#endif /* __EXP_OUTPUT_H__ */
