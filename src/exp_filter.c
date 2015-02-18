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

#include "exp_filter.h"
#include "exp_log.h"
#include "exp_regexp.h"
#include "exp_file.h"

#define DEFAULT_CAPACITY 128

static const char *dirs[] = {
     "/var/lib/exp/filters/",
     "/usr/local/exp/var/lib/filters/",
     "/opt/exp/var/lib/filters/"
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
};

static const char *impl_scrub(filter_impl_t *this, const char *line) {
     static char result[MAX_LINE_SIZE];
     int i, n = this->length;
     regexp_t *stopword;
     strncpy(result, line, MAX_LINE_SIZE);
     result[MAX_LINE_SIZE-1] = '\0';
     for (i = 0; i < n; i++) {
          stopword = this->stopwords[i];
          stopword->replace_all(stopword, "#", result);
     }
     return result;
}

static bool_t impl_bleach(filter_impl_t *this, const char *line) {
     const char *scrubbed = impl_scrub(this, line);
     return !strcmp("#", scrubbed);
}

static void add_regexp(filter_impl_t *this, regexp_t *regexp) {
     if (this->capacity == this->length) {
          if (this->capacity == 0) {
               this->capacity = DEFAULT_CAPACITY;
               this->stopwords = calloc(this->capacity, sizeof(regexp_t*));
          } else {
               this->capacity = this->capacity * 2;
               this->stopwords = realloc(this->stopwords, this->capacity);
          }
     }
     this->stopwords[this->length++] = regexp;
}

static void impl_init(filter_impl_t *this, const char *dir, const char *filename) {
     char *path = malloc(strlen(dir) + strlen(filename) + 2);
     file_t *file;
     line_t *line;
     regexp_t *regexp;
     sprintf(path, "%s/%s", dir, filename);
     file = new_file(this->log, debug, path);
     if (file != NULL) {
          line = file->get_lines(file);
          while (line != NULL) {
               regexp = new_regexp(this->log, line->buffer, 0, 0);
               if (regexp != NULL) {
                    add_regexp(this, regexp);
               }
          }
          file->free(file);
     }
     free(path);
}

static filter_t filter_impl_fn = {
     .scrub = (filter_scrub_fn)impl_scrub,
     .bleach = (filter_bleach_fn)impl_bleach,
};

filter_t *new_filter(logger_t log, const char *filename) {
     filter_impl_t *result = malloc(sizeof(filter_impl_t));
     const char *dir;
     result->fn = filter_impl_fn;
     result->log = log;
     result->length = 0;
     result->capacity = 0;
     for (dir = dirs[0]; dir != NULL; dir++) {
          impl_init(result, dir, filename);
     }
     return &(result->fn);
}
