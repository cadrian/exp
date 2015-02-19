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

#ifndef __EXP_H__
#define __EXP_H__

/**
 * @file
 * The main header for the ExP application.
 */

#include "cad_shared.h"

#define EXP_GRAND_VERSION 0
#define EXP_MAJOR_VERSION 0
#define EXP_MINOR_VERSION 1

/**
 * @addtogroup exp
 * @{
 */

/**
 * The boolean type.
 */
typedef enum {
     /** false */
     false=0,
     /** true */
     true
} bool_t;

/**
 * Find the numeric value of the short month name Jan...Dec return 1..12.
 *
 * @param[in] month the month name.
 */
int month_of(const char *month);

/**
 * @}
 */

#endif /* __EXP_H__ */
