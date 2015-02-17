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

#ifndef __EXP_ENTRY_LOG_C__
#define __EXP_ENTRY_LOG_C__

#include <stdlib.h>
#include <string.h>

#include "exp.h"

static char *months[] = {
     "",
     "Jan",
     "Feb",
     "Mar",
     "Apr",
     "May",
     "Jun",
     "Jul",
     "Aug",
     "Sep",
     "Oct",
     "Nov",
     "Dec",
};

static int month_of(const char *month) {
     int result = 0;
     int index;
     for (index = 1; result == 0 && index <= 12; index++) {
          if (!strcmp(months[index], month)) {
               result = index;
          }
     }
     return result;
}

static bool_t log_tally_logic(entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines) {
     return tally > tally_threshold;
}

#endif /* __EXP_ENTRY_LOG_C__ */
