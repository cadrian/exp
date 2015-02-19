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

#ifndef __EXP_LOG_H__
#define __EXP_LOG_H__

/**
 * @file
 * The header for inputs.
 */

#include "exp.h"

/**
 * @addtogroup exp
 * @{
 */

/**
 * The log levels
 */
typedef enum {
     /**
      * Warnings (default level)
      */
     warn=0,
     /**
      * Informations on how ExP runs
      */
     info,
     /**
      * Developer details, usually not useful
      */
     debug,
} level_t;

/**
 * A logger is a `printf`-like function to call
 *
 * @param[in] level the logging level
 * @param[in] format the message format
 * @param[in] ... the message arguments
 *
 * @return the length of the message after expansion
 */
typedef int (*logger_t) (level_t level, char *format, ...) __PRINTF__;

/**
 * Create a new logger
 *
 * @param[in] level the max level of logging
 */
logger_t new_logger(level_t level);

/**
 * @}
 */

#endif /* __EXP_LOG_H__ */
