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
 * This file contains the implementation of the entry factories.
 */

#include <string.h>
#include <cad_hash.h>

#include "exp_entry_factory.h"
#include "exp_log.h"

#define DEFAULT_CAPACITY 16

static entry_factory_t **list = NULL;
static size_t list_size = 0;
static size_t list_capacity = 0;
static cad_hash_t *factories = NULL;

static void resize_list() {
     size_t new_capacity;
     entry_factory_t **new_list;
     if (list_capacity == 0) {
          new_capacity = DEFAULT_CAPACITY;
          new_list = malloc(new_capacity * sizeof(entry_factory_t*));
     } else {
          new_capacity = list_capacity * 2;
          new_list = realloc(list, new_capacity * sizeof(entry_factory_t*));
     }
     list_capacity = new_capacity;
     list = new_list;
}

static int compare_factories(entry_factory_t **f1, entry_factory_t **f2) {
     int result = (*f2)->priority(*f2) - (*f1)->priority(*f1);
     if (result == 0) {
          result = strcmp((*f1)->get_name(*f1), (*f2)->get_name(*f2));
     }
     return result;
}

void sort_factories(logger_t log) {
     int i;
     entry_factory_t *f;
     qsort(list, list_size, sizeof(entry_factory_t*), (int(*)(const void*,const void*))compare_factories);
     log(debug, "Sorted entry factories:\n");
     for (i = 0; i < list_size; i++) {
          f = list[i];
          log(debug, "%2d: %s (%d)\n", i+1, f->get_name(f), f->priority(f));
     }
}

void register_factory(entry_factory_t *factory) {
     if (list_size == list_capacity) {
          resize_list();
     }
     list[list_size++] = factory;
     if (factories == NULL) {
          factories = cad_new_hash(stdlib_memory, cad_hash_strings);
     }
     factories->set(factories, factory->get_name(factory), factory);
}

size_t entry_factories_length(void) {
     return list_size;
}

entry_factory_t *entry_factory(int index) {
     return list[index];
}

entry_factory_t *entry_factory_named(const char *name) {
     return factories->get(factories, name);
}
