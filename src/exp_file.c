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

/**
 * @ingroup exp_input
 * @file
 *
 * This file contains the implementation of file reading algorithm.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "exp_file.h"

typedef struct file_impl_s file_impl_t;

struct file_impl_s {
     file_t fn;
     logger_t log;
     size_t length;
     line_t *lines;
};

static size_t impl_lines_count(file_impl_t *this) {
     return this->length;
}

static line_t *impl_lines(file_impl_t *this) {
     return this->lines;
}

static void impl_free(file_impl_t *this) {
     line_t *line = this->lines;
     line_t *next;
     while (line != NULL) {
          next = line->next;
          free(line);
          line = next;
     }
     free(this);
}

static file_t file_impl_fn = {
     .lines_count = (file_lines_count_fn)impl_lines_count,
     .lines = (file_lines_fn)impl_lines,
     .free = (file_free_fn)impl_free,
};

line_t *new_line(line_t *previous, size_t length, const char *content) {
     line_t *result = malloc(sizeof(line_t) + length + 1);
     char *line = (char*)result->buffer;
     if (previous != NULL) {
          previous->next = result;
     }
     result->next = NULL;
     result->length = length;
     memcpy(line, content, length + 1);
     line[length] = '\0';
     return result;
}

static void read_all_lines(file_impl_t *this, FILE *in) {
     static char buffer[MAX_LINE_SIZE];
     static char line[MAX_LINE_SIZE];

     line_t *first = NULL;
     line_t *last = NULL;

     size_t buffer_length;
     size_t line_length = 0;
     size_t buffer_index;
     bool_t line_too_long_flag = false;
     size_t count_lines_too_long = 0;

     this->length = 0;

     while ((buffer_length = fread(buffer, 1, MAX_LINE_SIZE, in)) > 0) {
          for (buffer_index = 0; buffer_index < buffer_length; buffer_index++) {
               if (buffer[buffer_index] == '\n') {
                    last = new_line(last, line_length, line);
                    if (first == NULL) {
                         first = last;
                    }
                    line_length = 0;
                    line_too_long_flag = false;
                    this->length++;
               } else if (line_length >= MAX_LINE_SIZE) {
                    if (!line_too_long_flag) {
                         this->log(info, "Truncating line %lu\n", (unsigned long)this->length);
                         count_lines_too_long++;
                         line_too_long_flag = true;
                         line[MAX_LINE_SIZE - 1] = '\0';
                    }
               } else {
                    line[line_length++] = buffer[buffer_index];
               }
          }
     }

     if (ferror(in)) {
          this->log(warn, "Error during read: %s\n", strerror(errno));
     } else if (line_length > 0) {
          last = new_line(last, line_length, line);
          if (first == NULL) {
               first = last;
          }
          this->length++;
     }

     if (count_lines_too_long > 0) {
          this->log(warn, "%lu line%s too long, truncated to %d characters\n", (unsigned long)count_lines_too_long, count_lines_too_long > 1 ? "s" : "", MAX_LINE_SIZE - 1);
     }
     this->log(debug, "Read %lu line%s\n", (unsigned long)this->length, this->length > 1 ? "s" : "");

     this->lines = first;
}

file_t *new_file(logger_t log, level_t error_level, const char *path) {
     file_impl_t *result = malloc(sizeof(file_impl_t));
     FILE *in;

     if (!strcmp("-", path)) {
          in = stdin;
          log(debug, "Using stdin\n");
     } else {
          in = fopen(path, "r");
          if (!in) {
               log(error_level, "%s: %s\n", strerror(errno), path);
               free(result);
               return NULL;
          }
          log(debug, "Opening file: %s\n", path);
     }

     result->fn = file_impl_fn;
     result->log = log;
     read_all_lines(result, in);

     if (in != stdin) {
          fclose(in);
     }

     return &(result->fn);
}
