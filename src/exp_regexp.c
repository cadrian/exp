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

#include "exp_log.h"
#include "exp_regexp.h"

typedef struct {
     regexp_t fn;
     logger_t log;
     pcre *re;
     pcre_extra *extra;
     char *regex;
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
   int status;
   if (this->sub != NULL) {
      pcre_free_substring(this->sub);
   }
   status = pcre_get_substring(this->string, this->subs, this->subslen, index, &(this->sub));
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

static match_impl_t *regexp_impl_match(regexp_impl_t *this, const char *string, int start, int length, int pcre_flags) {
     int subsmax = this->max_substrings * 3;
     match_impl_t *result = malloc(sizeof(match_impl_t) + sizeof(int) * subsmax);
     int status;

     result->fn = match_impl_fn;
     result->regexp = this;
     result->string = string;
     result->sub = NULL;
     result->subsmax = subsmax;
     status = pcre_exec(this->re, this->extra, string, length, start, pcre_flags, result->subs, subsmax);
     if (status < 0) {
          switch(status) {
          case PCRE_ERROR_NOMATCH     :                                                               break;
          case PCRE_ERROR_NULL        : this->log(warn, "Something was null\n");                      break;
          case PCRE_ERROR_BADOPTION   : this->log(warn, "A bad option was passed\n");                 break;
          case PCRE_ERROR_BADMAGIC    : this->log(warn, "Magic number bad (compiled re corrupt?)\n"); break;
          case PCRE_ERROR_UNKNOWN_NODE: this->log(warn, "Something kooky in the compiled re\n");      break;
          case PCRE_ERROR_NOMEMORY    : this->log(warn, "Ran out of memory\n");                       break;
          case PCRE_ERROR_BADOFFSET   : this->log(warn, "Bad offset\n");                              break;
          default                     : this->log(warn, "PCRE error %d\n", status);                   break;
          }
          free(result);
          return NULL;
     }

     if (status == 0) {
          result->subslen = this->max_substrings;
     } else {
          result->subslen = status;
     }

     return result;
}

static void regexp_impl_replace_all(regexp_impl_t *this, const char *replace, char *string) {
     int len_string = strlen(string);
     int len_replace = strlen(replace);
     match_impl_t *match;
     int start, end, lost = 0, delta;
     char *whole = string;

     match = regexp_impl_match(this, string, 0, len_string, 0);
     while (match != NULL) {
          start = match->subs[0];
          end = match->subs[1];
          if (len_replace > end - start + lost) {
               this->log(warn, "string replacement longer that matched substring, cannot replace\n");
          } else {
               delta = end - start - len_replace;
               lost += delta;
               if (delta < 0) {
                    bcopy(string + end, string + end - delta, len_string - delta);
               }
               memcpy(string + start, replace, len_replace);
               if (delta > 0) {
                    bcopy(string + end, string + end - delta, len_string - delta);
               }
               len_string -= delta;
               string[len_string] = '\0';
               end = start + len_replace;
          }
          match_impl_free(match);
          if (end >= len_string) {
               match = NULL;
          } else {
               len_string -= end;
               string += end;
               match = regexp_impl_match(this, string, 0, len_string, PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED);
          }
     }
}

static void regexp_impl_free(regexp_impl_t *this) {
     pcre_free(this->re);
     if (this->extra != NULL) {
          pcre_free_study(this->extra);
     }
     free(this->regex);
     free(this);
}

static regexp_t regexp_impl_fn = {
     .match = (regexp_match_fn)regexp_impl_match,
     .replace_all = (regexp_replace_all_fn)regexp_impl_replace_all,
     .free = (regexp_free_fn)regexp_impl_free,
};

regexp_t *new_regexp(logger_t log, const char *regex, int pcre_flags) {
     regexp_impl_t *result = malloc(sizeof(regexp_impl_t));
     const char *pcre_error_string;
     int pcre_error_index;
     int max_substrings;
     int capturecount, namecount;

     result->re = pcre_compile(regex, pcre_flags, &pcre_error_string, &pcre_error_index, NULL);
     if (result->re == NULL) {
          free(result);
          log(warn, "Error while compiling regexp %s\n%s at %d\n", regex, pcre_error_string, pcre_error_index);
          return NULL;
     }

     result->extra = pcre_study(result->re, PCRE_STUDY_JIT_COMPILE, &pcre_error_string);
     if (pcre_error_string != NULL) {
          pcre_free(result->re);
          free(result);
          log(warn, "Error while optimizing regexp %s\n%s\n", regex, pcre_error_string);
          return NULL;
     }

     pcre_fullinfo(result->re, result->extra, PCRE_INFO_CAPTURECOUNT, &capturecount);
     pcre_fullinfo(result->re, result->extra, PCRE_INFO_NAMECOUNT, &namecount);
     max_substrings = capturecount + namecount;

     result->fn = regexp_impl_fn;
     result->log = log;
     result->regex = strdup(regex);
     result->max_substrings = max_substrings + 1;

     return &(result->fn);
}
