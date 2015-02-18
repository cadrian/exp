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

#ifndef __EXP_ENTRY_H__
#define __EXP_ENTRY_H__

#include "exp.h"
#include "exp_log.h"

typedef struct entry_s entry_t;

typedef int (*entry_year_fn)(entry_t *this);
typedef int (*entry_month_fn)(entry_t *this);
typedef int (*entry_day_fn)(entry_t *this);
typedef int (*entry_hour_fn)(entry_t *this);
typedef int (*entry_minute_fn)(entry_t *this);
typedef int (*entry_second_fn)(entry_t *this);
typedef const char *(*entry_host_fn)(entry_t *this);
typedef const char *(*entry_daemon_fn)(entry_t *this);
typedef const char *(*entry_logline_fn)(entry_t *this);

struct entry_s {
     entry_year_fn    year   ;
     entry_month_fn   month  ;
     entry_day_fn     day    ;
     entry_hour_fn    hour   ;
     entry_minute_fn  minute ;
     entry_second_fn  second ;
     entry_host_fn    host   ;
     entry_daemon_fn  daemon ;
     entry_logline_fn logline;
};

#endif /* __EXP_ENTRY_H__ */
