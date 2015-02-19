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

#ifndef __EXP_ENTRY_H__
#define __EXP_ENTRY_H__

/**
 * @ingroup exp_input
 * @file
 *
 * An *entry* is a line and its analysis. The analysis depends on the entry factory, in other words the "type" of the line.
 */

#include "exp.h"
#include "exp_log.h"

/**
 * @addtogroup exp_input
 * @{
 */

/**
 * The entry interface.
 */
typedef struct entry_s entry_t;

/**
 * Get the name of the entry, which is in fact the name of its factory.
 *
 * @param[in] this the target entry
 *
 * @return the name of its factory; in other words: its type
 */
typedef const char *(*entry_get_name_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the year of the entry
 */
typedef int (*entry_year_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the month of the entry
 */
typedef int (*entry_month_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the day of the entry
 */
typedef int (*entry_day_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the hour of the entry
 */
typedef int (*entry_hour_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the minute of the entry
 */
typedef int (*entry_minute_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the second of the entry
 */
typedef int (*entry_second_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the host of the entry
 */
typedef const char *(*entry_host_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the daemon of the entry
 */
typedef const char *(*entry_daemon_fn)(entry_t *this);

/**
 * @param[in] this the target entry
 * @return the logline of the entry
 */
typedef const char *(*entry_logline_fn)(entry_t *this);

struct entry_s {
     /**
      * @see entry_get_name_fn
      */
     entry_get_name_fn get_name;
     /**
      * @see entry_year_fn
      */
     entry_year_fn    year   ;
     /**
      * @see entry_month_fn
      */
     entry_month_fn   month  ;
     /**
      * @see entry_day_fn
      */
     entry_day_fn     day    ;
     /**
      * @see entry_hour_fn
      */
     entry_hour_fn    hour   ;
     /**
      * @see entry_minute_fn
      */
     entry_minute_fn  minute ;
     /**
      * @see entry_second_fn
      */
     entry_second_fn  second ;
     /**
      * @see entry_host_fn
      */
     entry_host_fn    host   ;
     /**
      * @see entry_daemon_fn
      */
     entry_daemon_fn  daemon ;
     /**
      * @see entry_logline_fn
      */
     entry_logline_fn logline;
};

/**
 * @}
 */

#endif /* __EXP_ENTRY_H__ */
