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

/**
 * @file
 * The header for entry factories.
 */

#include "exp_entry.h"
#include "exp_file.h"

/**
 * @addtogroup exp_input
 * @{
 */

/**
 * The entry factory interface.
 */
typedef struct entry_factory_s entry_factory_t;

/**
 * Get the name of the factory
 *
 * @param[in] this the target entry factory
 *
 * @return the name of the factory
 */
typedef const char *(*entry_factory_get_name_fn)(entry_factory_t *this);

/**
 * Get the priority of the factory
 *
 * @param[in] this the target entry factory
 *
 * @return the priority of the factory
 */
typedef int (*entry_factory_priority_fn)(entry_factory_t *this);

/**
 * The "tally logic" that allows to select the most appropriate factory.
 *
 * @param[in] this the target entry factory
 * @param[in] tally the tally for this factory
 * @param[in] tally_threshold the tally threshold
 * @param[in] max_sample_lines the number of sampled lines
 *
 * @return `true` if the factory considers being a good representation of the sampled lines, `false` otherwise
 */
typedef bool_t (*entry_factory_tally_logic_fn)(entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines);

/**
 * Is the given line understandable by the factory? The answer to that question is used to set the tally.
 *
 * @param[in] this the target entry factory
 * @param[in] line the line to check
 *
 * @return `true` if the line is of a type this factory understands; `false` otherwise
 */
typedef bool_t (*entry_factory_is_type_fn)(entry_factory_t *this, line_t *line);

/**
 * Creates a new entry corresponding to the analysis of the given line.
 *
 * @param[in] this the target entry factory
 * @param[in] line the line to analyze
 *
 * @return the analyzed line
 */
typedef entry_t *(*entry_factory_new_entry_fn)(entry_factory_t *this, line_t *line);

typedef void (*entry_factory_set_extradirs_fn)(entry_factory_t *this, const char **extradirs);

struct entry_factory_s {
     /**
      * @see entry_factory_get_name_fn
      */
     entry_factory_get_name_fn get_name;
     /**
      * @see entry_factory_priority_fn
      */
     entry_factory_priority_fn priority;
     /**
      * @see entry_factory_tally_logic_fn
      */
     entry_factory_tally_logic_fn tally_logic;
     /**
      * @see entry_factory_is_type_fn
      */
     entry_factory_is_type_fn is_type;
     /**
      * @see entry_factory_new_entry_fn
      */
     entry_factory_new_entry_fn new_entry;
     /**
      * @see entry_factory_set_extradirs_fn
      */
     entry_factory_set_extradirs_fn set_extradirs;
};

/**
 * Register all the known factories.
 *
 * The implementation of that method is generated.
 *
 * @param[in] log the logger
 */
void register_all_factories(logger_t log);

/**
 * Sort all the known priorities by descending priority and ascending name.
 *
 * @param[in] log the logger
 */
void sort_factories(logger_t log);

/**
 * Register a factory.
 *
 * This method is called by [register_all_factories](@ref register_all_factories).
 */
void register_factory(entry_factory_t *factory);

/**
 * The number of registered factories.
 *
 * @return the number of registered factories
 */
size_t entry_factories_length(void);

/**
 * The *index*-th factory.
 *
 * @param[in] index the index of the factory to return
 *
 * @return the *index*-th factory
 */
entry_factory_t *entry_factory(int index);

/**
 * The factory by its name.
 *
 * @param[in] name the name of the factory to return
 *
 * @return the factory named `name`.
 */
entry_factory_t *entry_factory_named(const char *name);

/**
 * @}
 */

#endif /* __EXP_ENTRY_FACTORY_H__ */
