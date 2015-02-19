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

#ifndef __EXP_FILE_H__
#define __EXP_FILE_H__

#include "exp.h"
#include "exp_log.h"

#define MAX_LINE_SIZE 4096

typedef struct line_s line_t;
struct line_s {
     line_t *next;
     size_t length;
     const char buffer[0];
};

typedef struct file_s file_t;

typedef size_t (*file_lines_count_fn)(file_t *this);
typedef line_t *(*file_lines_fn)(file_t *this);
typedef void (*file_free_fn)(file_t *this);

struct file_s {
     file_lines_count_fn lines_count;
     file_lines_fn lines;
     file_free_fn free;
};

file_t *new_file(logger_t log, level_t error_level, const char *path);

#endif /* __EXP_FILE_H__ */
