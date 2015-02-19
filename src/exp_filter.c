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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "exp_filter.h"
#include "exp_log.h"
#include "exp_regexp.h"
#include "exp_file.h"

#define DEFAULT_CAPACITY 128

static const char *dirs[] = {
     "/var/lib/exp/filters/",
     "/usr/local/exp/var/lib/filters/",
     "/opt/exp/var/lib/filters/",
     "/var/lib/petit/filters/",
     "/usr/local/petit/var/lib/filters/",
     "/opt/petit/var/lib/filters/",
     NULL
};

typedef struct filter_impl_s filter_impl_t;

struct filter_impl_s {
     filter_t fn;
     logger_t log;
     size_t length;
     size_t capacity;
     regexp_t **stopwords;
     char **replacements;
};

static const char *impl_scrub(filter_impl_t *this, entry_t *entry) {
     static char result[MAX_LINE_SIZE];
     int i, n = this->length;
     regexp_t *stopword;
     const char *daemon, *logline;
     daemon = entry->daemon(entry);
     logline = entry->logline(entry);
     if (logline == NULL) {
          strcpy(result, "#");
     } else {
          if (daemon == NULL) {
               strncpy(result, logline, MAX_LINE_SIZE);
               result[MAX_LINE_SIZE-1] = '\0';
          } else {
               snprintf(result, MAX_LINE_SIZE, "%s %s", daemon, logline);
          }
          for (i = 0; i < n; i++) {
               stopword = this->stopwords[i];
               stopword->replace_all(stopword, this->replacements[i], result);
          }
     }
     return result;
}

static bool_t impl_bleach(filter_impl_t *this, entry_t *entry) {
     const char *scrubbed = impl_scrub(this, entry);
     return !strcmp("#", scrubbed);
}

static void add_regexp(filter_impl_t *this, regexp_t *regexp, const char *replacement) {
     if (this->capacity == this->length) {
          if (this->capacity == 0) {
               this->capacity = DEFAULT_CAPACITY;
               this->stopwords = malloc(this->capacity * sizeof(regexp_t*));
               this->replacements = malloc(this->capacity * sizeof(char*));
          } else {
               this->capacity = this->capacity * 2;
               this->stopwords = realloc(this->stopwords, this->capacity * sizeof(regexp_t*));
               this->replacements = realloc(this->replacements, this->capacity * sizeof(char*));
          }
     }
     this->stopwords[this->length] = regexp;
     this->replacements[this->length] = strdup(replacement);
     this->length++;
}

static char *split(char **regexp) {
     char *string = *regexp;
     char *result = NULL;
     char separator = *string;
     char c;
     int state = 0;

     if (separator < ' ' || (separator >= '0' && separator <= '9') || (separator >= 'A' && separator <= 'Z') || (separator >= 'a' && separator <= 'z')) {
          // not a separator
     } else {
          while (state >= 0) {
               string++;
               c = *string;
               if (c == '\0') {
                    break;
               }
               switch(state) {
               case 0:
                    if (c == '\\') {
                         state = 1;
                    } else if (c == separator) {
                         *string = '\0';
                         result = string + 1;
                         (*regexp)++;
                         state = 2;
                    }
                    break;
               case 1:
                    state = 0;
                    break;
               case 2:
                    if (c == '\\') {
                         state = 3;
                    } else if (c == separator) {
                         *string = '\0';
                         state = -1;
                    }
                    break;
               case 3:
                    state = 2;
                    break;
               }
          }
     }

     return result;
}

static void impl_extend_(filter_impl_t *this, const char *dir, const char *filename, const char *replacement) {
     char *path = malloc(strlen(dir) + strlen(filename) + 1);
     file_t *file;
     line_t *line;
     regexp_t *regexp;
     bool_t has_replacement = replacement != NULL;
     static char re_[MAX_LINE_SIZE];
     char *re;

     sprintf(path, "%s%s", dir, filename);
     file = new_file(this->log, debug, path);
     if (file != NULL) {
          line = file->lines(file);
          while (line != NULL) {
               if (has_replacement) {
                    re = (char*)line->buffer;
               } else {
                    strncpy(re_, line->buffer, line->length);
                    re_[line->length] = '\0';
                    re = re_;
                    replacement = split(&re);
                    if (replacement == NULL) {
                         replacement = "#";
                    }
               }
               regexp = new_regexp(this->log, re, 0);
               if (regexp != NULL) {
                    add_regexp(this, regexp, replacement);
               }
               line = line->next;
          }
          file->free(file);
     }
     free(path);
}

static void impl_extend(filter_impl_t *this, const char *filename, const char *replacement) {
     const char *dir;
     int i;
     for (i = 0; (dir = dirs[i]) != NULL; i++) {
          impl_extend_(this, dir, filename, replacement);
     }
}

static filter_t filter_impl_fn = {
     .extend = (filter_extend_fn)impl_extend,
     .scrub = (filter_scrub_fn)impl_scrub,
     .bleach = (filter_bleach_fn)impl_bleach,
};

filter_t *new_filter(logger_t log) {
     filter_impl_t *result = malloc(sizeof(filter_impl_t));

     result->fn = filter_impl_fn;
     result->log = log;
     result->length = result->capacity = 0;
     result->stopwords = NULL;

     return &(result->fn);
}
