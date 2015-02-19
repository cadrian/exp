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

#ifndef __EXP_FILTER_H__
#define __EXP_FILTER_H__

/**
 * @file
 * The header for filters.
 */

#include "exp.h"
#include "exp_log.h"
#include "exp_entry.h"

/**
 * @addtogroup exp_output
 * @{
 */

/**
 * The filter interface.
 */
typedef struct filter_s filter_t;

/**
 * Add all the patterns from the given file.
 *
 * If the given *replacement* is provided, those patterns are meant to be replaced that.
 *
 * Otherwise (if *replacement* is `null`), the filter looks for
 * replacements into the file itself. In that case, the lines are a
 * compound of the regexp and its replacent, separated by a character
 * (also at the begin and end of the line, usually a slash
 * `sed`-like).
 *
 * @param[in] this the target filter
 * @param[in] filename the name of the file
 * @param[in] replacement the replacement string, or `null` to discover it into the file
 */
typedef void (*filter_extend_fn)(filter_t *this, const char *filename, const char *replacement);

/**
 * Scrub patterns out of the given line, replacing them by their
 * respective replacements (see [extend](@ref filter_extend_fn)).
 *
 * The returned string is a static buffer.
 *
 * @param[in] this the target filter
 * @param[in] line the line to scrub
 *
 * @return the scrubbed line
 */
typedef const char *(*filter_scrub_fn)(filter_t *this, const char *line);

/**
 * See if the line can be reduced to nothing by intensive [scrubbing](@ref filter_scrub_fn)
 *
 * @param[in] this the target filter
 * @param[in] line the line to check
 *
 * @return `true` if the line can be scrubbed to oblivion, `false` otherwise
 */
typedef bool_t (*filter_bleach_fn)(filter_t *this, const char *line);

struct filter_s {
     /**
      * @see filter_extend_fn
      */
     filter_extend_fn extend;
     /**
      * @see filter_scrub_fn
      */
     filter_scrub_fn scrub;
     /**
      * @see filter_bleach_fn
      */
     filter_bleach_fn bleach;
};

/**
 * Create a new filter.
 *
 * @param[in] log the logger
 *
 * @return the new filter
 */
filter_t *new_filter(logger_t log);

/**
 * @}
 */

#endif /* __EXP_FILTER_H__ */
