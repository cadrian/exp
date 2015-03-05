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
#include <cad_array.h>

#include "exp_file.h"

typedef struct file_impl_s file_impl_t;

struct file_impl_s {
     file_t fn;
     logger_t log;
     size_t size;
     cad_array_t *lines;
};

static size_t impl_lines_count(file_impl_t *this) {
     return this->lines->count(this->lines);
}

static line_t *impl_line(file_impl_t *this, int index) {
     return *(line_t **)this->lines->get(this->lines, index);
}

static size_t impl_size(file_impl_t *this) {
     return this->size;
}

static void impl_free(file_impl_t *this) {
     this->lines->free(this->lines);
     free(this);
}

static file_t file_impl_fn = {
     .lines_count = (file_lines_count_fn)impl_lines_count,
     .line = (file_line_fn)impl_line,
     .size = (file_size_fn)impl_size,
     .free = (file_free_fn)impl_free,
};

line_t *new_line(size_t length, const char *content) {
     line_t *result = malloc(sizeof(line_t) + length + 1);
     char *line = (char*)result->buffer;
     result->length = length;
     memcpy(line, content, length + 1);
     line[length] = '\0';
     return result;
}

static void read_all_lines(file_impl_t *this, FILE *in) {
     static char buffer[MAX_LINE_SIZE];
     static char linebuf[MAX_LINE_SIZE];

     line_t *line = NULL;

     size_t buffer_length;
     size_t line_length = 0;
     size_t buffer_index;
     bool_t line_too_long_flag = false;
     size_t count_lines_too_long = 0;
     int count = 0;

     this->lines = cad_new_array(stdlib_memory, sizeof(line_t *));
     this->size = 0;

     while ((buffer_length = fread(buffer, 1, MAX_LINE_SIZE, in)) > 0) {
          for (buffer_index = 0; buffer_index < buffer_length; buffer_index++) {
               if (buffer[buffer_index] == '\n') {
                    line = new_line(line_length, linebuf);
                    this->lines->insert(this->lines, count++, &line);
                    this->size += line_length;
                    line_length = 0;
                    line_too_long_flag = false;
               } else if (line_length >= MAX_LINE_SIZE) {
                    if (!line_too_long_flag) {
                         this->log(info, "Truncating line %lu\n", (unsigned long)count);
                         count_lines_too_long++;
                         line_too_long_flag = true;
                         linebuf[MAX_LINE_SIZE - 1] = '\0';
                    }
               } else {
                    linebuf[line_length++] = buffer[buffer_index];
               }
          }
     }

     if (ferror(in)) {
          this->log(warn, "Error during read: %s\n", strerror(errno));
     } else if (line_length > 0) {
          line = new_line(line_length, linebuf);
          this->lines->insert(this->lines, count++, &line);
          this->size += line_length;
     }

     this->size += count; /* count 1 per EOL */

     if (count_lines_too_long > 0) {
          this->log(warn, "%lu line%s too long, truncated to %d characters\n", (unsigned long)count_lines_too_long, count_lines_too_long > 1 ? "s" : "", MAX_LINE_SIZE - 1);
     }
     this->log(debug, "Read %lu line%s\n", (unsigned long)count, count > 1 ? "s" : "");
}

file_t *new_file(logger_t log, level_t error_level, const char *path) {
     file_impl_t *result = malloc(sizeof(file_impl_t));
     FILE *in;

     if (!strcmp("-", path)) {
          in = stdin;
          log(info, "Reading stdin\n");
     } else {
          in = fopen(path, "r");
          if (!in) {
               log(error_level, "%s: %s\n", strerror(errno), path);
               free(result);
               return NULL;
          }
          log(info, "Reading file: %s\n", path);
     }

     result->fn = file_impl_fn;
     result->log = log;
     read_all_lines(result, in);

     if (in != stdin) {
          fclose(in);
     }

     return &(result->fn);
}
