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

#include "exp_entry_factory.h"
#include "exp_log.h"

#define DEFAULT_CAPACITY 16

static entry_factory_t **list = NULL;
static size_t list_size = 0;
static size_t list_capacity = 0;

static void resize_list() {
     size_t new_capacity;
     entry_factory_t **new_list;
     if (list_capacity == 0) {
          new_capacity = DEFAULT_CAPACITY;
          new_list = calloc(new_capacity, sizeof(entry_factory_t*));
     } else {
          new_capacity = list_capacity * 2;
          new_list = realloc(list, new_capacity);
     }
     list_capacity = new_capacity;
     list = new_list;
}

void register_factory(entry_factory_t *factory) {
     if (list_size == list_capacity) {
          resize_list();
     }
     list[list_size++] = factory;
}

size_t entry_factories_length(void) {
     return list_size;
}

entry_factory_t *entry_factory(int index) {
     return list[index];
}
