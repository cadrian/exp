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

#ifndef __EXP_ENTRY_FACTORY_H__
#define __EXP_ENTRY_FACTORY_H__

#include "exp_entry.h"

typedef struct entry_factory_s entry_factory_t;

typedef const char *(*get_name_fn)(entry_factory_t *this);
typedef bool_t (*tally_logic_fn)(entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines);
typedef bool_t (*is_type_fn)(entry_factory_t *this, line_t *line);
typedef entry_t *(*new_entry_fn)(entry_factory_t *this, line_t *line);

struct entry_factory_s {
     get_name_fn get_name;
     tally_logic_fn tally_logic;
     is_type_fn is_type;
     new_entry_fn new_entry;
};

void register_all_factories(logger_t log);
void register_factory(entry_factory_t *factory);

size_t entry_factories_length(void);
entry_factory_t *entry_factory(int index);

#endif /* __EXP_ENTRY_FACTORY_H__ */
