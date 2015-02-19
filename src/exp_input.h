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

#include "exp.h"
#include "exp_log.h"
#include "exp_entry.h"
#include "exp_entry_factory.h"

typedef struct input_s input_t;
typedef struct input_file_s input_file_t;

typedef input_file_t *(*input_parse_fn)(input_t *this, const char *filename);
typedef size_t (*input_files_length_fn)(input_t *this);
typedef input_file_t *(*input_file_fn)(input_t *this, int index);
typedef void (*input_sort_files_fn)(input_t *this);

struct input_s {
     input_parse_fn parse;
     input_files_length_fn files_length;
     input_file_fn file;
     input_sort_files_fn sort_files;
};

typedef entry_factory_t *(*input_file_get_factory_fn)(input_file_t *this);
typedef const char *(*input_file_get_name_fn)(input_file_t *this);
typedef size_t (*input_file_entries_length_fn)(input_file_t *this);
typedef entry_t *(*input_file_entry_fn)(input_file_t *this, int index);

struct input_file_s {
     input_file_get_factory_fn get_factory;
     input_file_get_name_fn get_name;
     input_file_entries_length_fn entries_length;
     input_file_entry_fn entry;
};

input_t *new_input(logger_t log);

#endif /* __EXP_INPUT_H__ */
