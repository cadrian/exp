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

#ifndef __EXP_REGEXP_H__
#define __EXP_REGEXP_H__

/**
 * @file
 * The header for regexps.
 *
 * Note: the underlying engine is PCRE, hence powerful regular
 * expressions (in particular, named groups are extensively used by
 * ExP).
 */

#include <pcre.h>
#if PCRE_MAJOR < 8
#error PCRE is too old!!!
#endif

/**
 * @addtogroup exp
 * @{
 */

#include "exp_log.h"

/**
 * The regexp interface.
 */
typedef struct regexp_s regexp_t;

/**
 * The match interface.
 */
typedef struct match_s match_t;

/**
 * Try to match the regular expression.
 *
 * @param[in] this the target regexp
 * @param[in] string the string to match
 * @param[in] start the start of the string to match
 * @param[in] length the length of the string to match
 * @param[in] pcre_flags PCRE flags for regexp execution
 *
 * @return the match, or `null` if the string does not match
 */
typedef match_t *(*regexp_match_fn)(regexp_t *this, const char *string, int start, int length, int pcre_flags);

/**
 * Replace all occurrences of the regular expressions in *string* by
 * the given *replace*.
 *
 * Note that replacements happen in-place in the given *string*. Some
 * attention is given not to widen the string, but it may be
 * shortened.
 *
 * @param[in] this the target regexp
 * @param[in] replace the replacement string
 * @param[in] string the string to be modified
 */
typedef void (*regexp_replace_all_fn)(regexp_t *this, const char *replace, char *string);

/**
 * Free the regexp
 *
 * @param[in] this the target regexp
 */
typedef void (*regexp_free_fn)(regexp_t *this);

struct regexp_s {
     /**
      * @see regexp_match_fn
      */
     regexp_match_fn match;
     /**
      * @see regexp_replace_all_fn
      */
     regexp_replace_all_fn replace_all;
     /**
      * @see regexp_free_fn
      */
     regexp_free_fn free;
};

/**
 * Return the *index*-th unnamed group
 *
 * @param[in] this the target match
 * @param[in] index the index of the unnamed group to return
 *
 * @return the *index*-th unnamed group
 */
typedef const char *(*regexp_match_substring_fn)(match_t *this, int index);

/**
 * Return the named group
 *
 * @param[in] this the target match
 * @param[in] name the name of the group
 *
 * @return the named group
 */
typedef const char *(*regexp_match_named_substring_fn)(match_t *this, const char *name);

/**
 * Free the match
 *
 * @param[in] this the target match
 */
typedef void (*regexp_match_free_fn)(match_t *this);

struct match_s {
     /**
      * @see regexp_match_substring_fn
      */
     regexp_match_substring_fn substring;
     /**
      * @see regexp_match_named_substring_fn
      */
     regexp_match_named_substring_fn named_substring;
     /**
      * @see regexp_match_free_fn
      */
     regexp_match_free_fn free;
};

/**
 * Create a new regular expression.
 *
 * @param[in] log the logger
 * @param[in] regex the regular expression
 * @param[in] pcre_flags PCRE flags for regexp compilation
 *
 * @return the compiled regexp
 */
regexp_t *new_regexp(logger_t log, const char *regex, int pcre_flags);

/**
 * @}
 */

#endif /* __EXP_REGEXP_H__ */
