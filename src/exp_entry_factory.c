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
#include <cad_array.h>
#include <cad_hash.h>

#include "exp_entry_factory.h"
#include "exp_log.h"

#define DEFAULT_CAPACITY 16

static cad_array_t *factories_list = NULL;
static cad_hash_t  *factories_map  = NULL;

static int compare_factories(entry_factory_t **f1, entry_factory_t **f2) {
     int result = (*f2)->priority(*f2) - (*f1)->priority(*f1);
     if (result == 0) {
          result = strcmp((*f1)->get_name(*f1), (*f2)->get_name(*f2));
     }
     return result;
}

void sort_factories(logger_t log) {
     int i, n;
     entry_factory_t *f;
     if (factories_list != NULL) {
          factories_list->sort(factories_list, (comparator_fn)compare_factories);
          n = factories_list->count(factories_list);
          log(debug, "Sorted entry factories:\n");
          for (i = 0; i < n; i++) {
               f = factories_list->get(factories_list, i);
               log(debug, "%2d: %s (%d)\n", i+1, f->get_name(f), f->priority(f));
          }
     }
}

void register_factory(entry_factory_t *factory) {
     if (factories_list == NULL) {
          factories_list = cad_new_array(stdlib_memory);
     }
     factories_list->insert(factories_list, factories_list->count(factories_list), factory);
     if (factories_map == NULL) {
          factories_map = cad_new_hash(stdlib_memory, cad_hash_strings);
     }
     factories_map->set(factories_map, factory->get_name(factory), factory);
}

size_t entry_factories_length(void) {
     return factories_list == NULL ? 0 : factories_list->count(factories_list);
}

entry_factory_t *entry_factory(int index) {
     return factories_list->get(factories_list, index);
}

entry_factory_t *entry_factory_named(const char *name) {
     return factories_map->get(factories_map, name);
}
