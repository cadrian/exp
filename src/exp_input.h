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

#ifndef __EXP_INPUT_H__
#define __EXP_INPUT_H__

#include "exp.h"
#include "exp_log.h"
#include "exp_entry.h"

typedef struct input_s input_t;

typedef void (*input_parse_fn)(input_t *this, char *filename);
typedef size_t (*input_entries_length_fn)(input_t *this);
typedef entry_t *(*input_entry_fn)(input_t *this, int index);

struct input_s {
     input_parse_fn parse;
     input_entries_length_fn entries_length;
     input_entry_fn entry;
};

input_t *new_input(logger_t log);

#endif /* __EXP_INPUT_H__ */
