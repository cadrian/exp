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

#ifndef __EXP_ENTRY_FACTORY_H__
#define __EXP_ENTRY_FACTORY_H__

#include "exp_entry.h"
#include "exp_file.h"

typedef struct entry_factory_s entry_factory_t;

typedef const char *(*entry_factory_get_name_fn)(entry_factory_t *this);
typedef bool_t (*entry_factory_tally_logic_fn)(entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines);
typedef bool_t (*entry_factory_is_type_fn)(entry_factory_t *this, line_t *line);
typedef entry_t *(*entry_factory_new_entry_fn)(entry_factory_t *this, line_t *line);

struct entry_factory_s {
     entry_factory_get_name_fn get_name;
     entry_factory_tally_logic_fn tally_logic;
     entry_factory_is_type_fn is_type;
     entry_factory_new_entry_fn new_entry;
};

void register_all_factories(logger_t log);
void register_factory(entry_factory_t *factory);

size_t entry_factories_length(void);
entry_factory_t *entry_factory(int index);

#endif /* __EXP_ENTRY_FACTORY_H__ */
