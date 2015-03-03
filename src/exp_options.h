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

#ifndef __EXP_OPTIONS_H__
#define __EXP_OPTIONS_H__

/**
 * @file
 * The header for inputs.
 */

#include "exp.h"

/**
 * @addtogroup exp_misc
 * @{
 */

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
 * The user-supplied options
 */
typedef struct {
     bool_t       filter;
     bool_t       fingerprint;
     char        *tick;
     bool_t       wide;
     sample_t     sample;
     const char **filter_extradirs;
     const char **fingerprint_extradirs;
     const char **factory_extradirs;
     int          year;
     bool_t       exp_mode;
     int          dev;
} options_t;

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
     bool_t factory_extradirs;
     bool_t year;
     bool_t exp_mode;
     bool_t dev;
} options_set_t;

/**
 * @}
 */

#endif /* __EXP_OPTIONS_H__ */
