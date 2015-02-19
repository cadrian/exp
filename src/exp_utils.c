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

#include <stdlib.h>
#include <string.h>

#include "exp.h"

/**
 * @ingroup exp
 * @file
 *
 * This file defines a few useful tools.
 */

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

int month_of(const char *month) {
     int result = 0;
     int index;
     for (index = 1; result == 0 && index <= 12; index++) {
          if (!strcmp(months[index], month)) {
               result = index;
          }
     }
     return result;
}
