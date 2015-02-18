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

#ifndef __EXP_REGEXP_H__
#define __EXP_REGEXP_H__

#include <pcre.h>

#include "exp_log.h"

typedef struct regexp_s regexp_t;
typedef struct match_s match_t;

typedef match_t *(*regexp_match_fn)(regexp_t *this, const char *string, int start, int length, int pcre_flags);
typedef void (*regexp_replace_all_fn)(regexp_t *this, const char *replace, char *string);
typedef void (*regexp_free_fn)(regexp_t *this);
typedef const char *(*regexp_match_substring_fn)(match_t *this, int index);
typedef const char *(*regexp_match_named_substring_fn)(match_t *this, const char *name);
typedef void (*regexp_match_free_fn)(match_t *this);

struct regexp_s {
     regexp_match_fn match;
     regexp_replace_all_fn replace_all;
     regexp_free_fn free;
};

struct match_s {
     regexp_match_substring_fn substring;
     regexp_match_named_substring_fn named_substring;
     regexp_match_free_fn free;
};

regexp_t *new_regexp(logger_t log, const char *regex, int pcre_flags);

#endif /* __EXP_REGEXP_H__ */
