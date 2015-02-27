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

#ifndef __EXP_INPUT_H__
#define __EXP_INPUT_H__

/**
 * @file
 * The header for inputs.
 */

#include "exp.h"
#include "exp_log.h"
#include "exp_entry.h"
#include "exp_entry_factory.h"

/**
 * @addtogroup exp_input
 * @{
 */

/**
 * The input interface.
 */
typedef struct input_s input_t;

/**
 * The input file interface.
 */
typedef struct input_file_s input_file_t;

/**
 * Parse the logfile given by its *filename* and returns the corresponding input file.
 *
 * @param[in] this the target input
 * @param[in] filename the name of the logfile
 *
 * @return the new input file, or `null` on error
 */
typedef input_file_t *(*input_parse_fn)(input_t *this, const char *filename);

/**
 * Get the number of successfully [parsed](@ref input_parse_fn) files
 *
 * @param[in] this the target input
 *
 * @return the number of parsed files
 */
typedef size_t (*input_files_length_fn)(input_t *this);

/**
 * Get the *index*-th input file.
 *
 * @param[in] this the target input
 * @param[in] index the index of the input file to return
 *
 * @return the *index*-th input file
 */
typedef input_file_t *(*input_file_fn)(input_t *this, int index);

/**
 * Sort the input file by descending order of [size](@ref input_file_size_fn).
 *
 * @param[in] this the target input
 */
typedef void (*input_sort_files_fn)(input_t *this);

struct input_s {
     /**
      * @see input_parse_fn
      */
     input_parse_fn parse;
     /**
      * @see input_files_length_fn
      */
     input_files_length_fn files_length;
     /**
      * @see input_file_fn
      */
     input_file_fn file;
     /**
      * @see input_sort_files_fn
      */
     input_sort_files_fn sort_files;
};

/**
 * Get the factory corresponding to the input file.
 *
 * That factory was the one used to analyze its lines. In other words,
 * all the entries in that file share that same factory.
 *
 * @param[in] this the target input file
 *
 * @return the entry factory
 */
typedef entry_factory_t *(*input_file_get_factory_fn)(input_file_t *this);

/**
 * Get the file name
 *
 * @param[in] this the target input file
 *
 * @return the file name
 */
typedef const char *(*input_file_get_name_fn)(input_file_t *this);

/**
 * Get the number of entries (i.e. analyzed lines) in the input file.
 *
 * @param[in] this the target input file
 *
 * @return the number of lines
 */
typedef size_t (*input_file_entries_length_fn)(input_file_t *this);

/**
 * Get the size of the input file.
 *
 * @param[in] this the target input file
 *
 * @return the size of the input file
 */
typedef size_t (*input_file_size_fn)(input_file_t *this);

/**
 * Get the *index*-th analyzed line.
 *
 * @param[in] this the target input file
 * @param[in] index the index of the line to return
 *
 * @return the *index*-th entry
 */
typedef entry_t *(*input_file_entry_fn)(input_file_t *this, int index);

struct input_file_s {
     /**
      * @see input_file_get_factory_fn
      */
     input_file_get_factory_fn get_factory;
     /**
      * @see input_file_get_name_fn
      */
     input_file_get_name_fn get_name;
     /**
      * @see input_file_entries_length_fn
      */
     input_file_entries_length_fn entries_length;
     /**
      * @see input_file_size_fn
      */
     input_file_size_fn size;
     /**
      * @see input_file_entry_fn
      */
     input_file_entry_fn entry;
};

/**
 * Create a new input
 *
 * @param[in] log the logger
 *
 * @return the new input
 */
input_t *new_input(logger_t log);

/**
 * @}
 */

#endif /* __EXP_INPUT_H__ */
