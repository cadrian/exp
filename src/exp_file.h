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

/**
 * @file
 * The header for files.
 */

#include "exp.h"
#include "exp_log.h"

/**
 * @addtogroup exp_input
 * @{
 */

#define MAX_LINE_SIZE 4096

/**
 * The line structure.
 */
typedef struct line_s line_t;

struct line_s {
     /**
      * A link to the next line
      */
     line_t *next;
     /**
      * The length of the line
      */
     size_t length;
     /**
      * The line content
      */
     const char buffer[0];
};

/**
 * The file interface.
 */
typedef struct file_s file_t;

/**
 * Get the number of lines in the file.
 *
 * @param[in] this the target file
 *
 * @return the number of lines
 */
typedef size_t (*file_lines_count_fn)(file_t *this);

/**
 * Get the size of the file.
 *
 * @param[in] this the target file
 *
 * @return the size
 */
typedef size_t (*file_size_fn)(file_t *this);

/**
 * Get all the file lines.
 *
 * @param[in] this the target file
 *
 * @return a pointer to the first line
 */
typedef line_t *(*file_lines_fn)(file_t *this);

/**
 * Free the file
 *
 * @param[in] this the target file
 */
typedef void (*file_free_fn)(file_t *this);

struct file_s {
     /**
      * @see file_lines_count_fn
      */
     file_lines_count_fn lines_count;
     /**
      * @see file_lines_fn
      */
     file_lines_fn lines;
     /**
      * @see file_size_fn
      */
     file_size_fn size;
     /**
      * @see file_free_fn
      */
     file_free_fn free;
};

/**
 * Create a new file
 *
 * @param[in] log the logger
 * @param[in] error_level the level to log file errors
 * @param[in] path the file path
 *
 * @return the new file, or `null` on error
 */
file_t *new_file(logger_t log, level_t error_level, const char *path);

/**
 * Create a new line. The content is copied to the line buffer.
 *
 * @param[in] previous the previous line
 * @param[in] length the length of the line
 * @param[in] content the content of the line
 */
line_t *new_line(line_t *previous, size_t length, const char *content);

/**
 * @}
 */

#endif /* __EXP_FILE_H__ */
