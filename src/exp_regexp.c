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
#include <stdio.h>
#include <string.h>

#include "exp_regexp.h"

typedef struct {
     regexp_t fn;
     pcre *re;
     pcre_extra *extra;
     const char *regex;
     int max_substrings;
} regexp_impl_t;

typedef struct {
     match_t fn;
     regexp_impl_t *regexp;
     const char *string;
     const char *sub;
     int subsmax;
     int subslen;
     int subs[0];
} match_impl_t;

static const char *match_impl_substring(match_impl_t *this, int index) {
     int status = pcre_get_substring(this->string, this->subs, this->subslen, index, &(this->sub));
     if (status < 0) {
          return NULL;
     }
     return this->sub;
}

static const char *match_impl_named_substring(match_impl_t *this, const char *name) {
     int status = pcre_get_named_substring(this->regexp->re, this->string, this->subs, this->subslen, name, &(this->sub));
     if (status < 0) {
          return NULL;
     }
     return this->sub;
}

static void match_impl_free(match_impl_t *this) {
     if (this->sub != NULL) {
          pcre_free_substring(this->sub);
     }
     free(this);
}

static match_t match_impl_fn = {
     .substring = (regexp_match_substring_fn)match_impl_substring,
     .named_substring = (regexp_match_named_substring_fn)match_impl_named_substring,
     .free = (regexp_match_free_fn)match_impl_free,
};

static match_t *regexp_impl_match(regexp_impl_t *this, const char *string, int pcre_flags) {
     int subsmax = this->max_substrings * 3;
     match_impl_t *result = malloc(sizeof(match_impl_t) + sizeof(int) * subsmax);
     int status;

     result->fn = match_impl_fn;
     result->string = string;
     result->sub = NULL;
     result->subsmax = subsmax;
     status = pcre_exec(this->re, this->extra, string, strlen(string), 0, pcre_flags, result->subs, subsmax);
     if (status < 0) {
          switch(status) {
          case PCRE_ERROR_NOMATCH     :                                                               break;
          case PCRE_ERROR_NULL        : fprintf(stderr, "Something was null\n");                      break;
          case PCRE_ERROR_BADOPTION   : fprintf(stderr, "A bad option was passed\n");                 break;
          case PCRE_ERROR_BADMAGIC    : fprintf(stderr, "Magic number bad (compiled re corrupt?)\n"); break;
          case PCRE_ERROR_UNKNOWN_NODE: fprintf(stderr, "Something kooky in the compiled re\n");      break;
          case PCRE_ERROR_NOMEMORY    : fprintf(stderr, "Ran out of memory\n");                       break;
          default                     : fprintf(stderr, "Unknown error\n");                           break;
          }
          free(result);
          return NULL;
     }

     if (status == 0) {
          result->subslen = this->max_substrings;
     } else {
          result->subslen = status;
     }

     return &(result->fn);
}

static void regexp_impl_free(regexp_impl_t *this) {
     pcre_free(this->re);
     if (this->extra != NULL) {
          pcre_free_study(this->extra);
     }
     free(this);
}

static regexp_t regexp_impl_fn = {
     .match = (regexp_match_fn)regexp_impl_match,
     .free = (regexp_free_fn)regexp_impl_free,
};

regexp_t *new_regexp(const char *regex, int pcre_flags, size_t max_substrings) {
     regexp_impl_t *result = malloc(sizeof(regexp_impl_t));
     const char *pcre_error_string;
     int pcre_error_index;

     result->fn = regexp_impl_fn;
     result->regex = regex;
     result->max_substrings = max_substrings;

     result->re = pcre_compile(regex, pcre_flags, &pcre_error_string, &pcre_error_index, NULL);
     if (result->re == NULL) {
          free(result);
          fprintf(stderr, "Error while compiling regexp %s\n%s at %d\n", regex, pcre_error_string, pcre_error_index);
          return NULL;
     }

     result->extra = pcre_study(result->re, PCRE_STUDY_JIT_COMPILE, &pcre_error_string);
     if (pcre_error_string != NULL) {
          pcre_free(result->re);
          free(result);
          fprintf(stderr, "Error while optimizing regexp %s\n%s\n", regex, pcre_error_string);
          return NULL;
     }

     return &(result->fn);
}
