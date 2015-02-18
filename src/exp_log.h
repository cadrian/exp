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

#ifndef __EXP_LOG_H__
#define __EXP_LOG_H__

#include "exp.h"

typedef enum {
   warn=0,
   info,
   debug,
} level_t;

typedef int (*logger_t) (level_t level, char *format, ...) __PRINTF__;

logger_t new_logger(level_t level);

#endif /* __EXP_LOG_H__ */
