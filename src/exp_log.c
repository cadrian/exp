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

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "exp_log.h"

static void taglog(level_t level) {
   struct timeval tm;
   char buffer[20];
   static char *tag[] = {
      "WARN ",
      "INFO ",
      "DEBUG"
   };
   gettimeofday(&tm, NULL);
   strftime(buffer, 20, "%Y/%m/%d %H:%M:%S", localtime(&tm.tv_sec));
   fprintf(stderr, "%s.%06ld [%s] ", buffer, tm.tv_usec, tag[level]);
}

static int warn_logger(level_t level, char *format, ...) {
   int result = 0;
   va_list arg;
   if(level <= warn) {
      va_start(arg, format);
      taglog(level);
      result = vfprintf(stderr, format, arg);
      va_end(arg);
   }
   return result;
}

static int info_logger(level_t level, char *format, ...) {
   int result = 0;
   va_list arg;
   if(level <= info) {
      va_start(arg, format);
      taglog(level);
      result = vfprintf(stderr, format, arg);
      va_end(arg);
   }
   return result;
}

static int debug_logger(level_t level, char *format, ...) {
   int result = 0;
   va_list arg;
   if(level <= debug) {
      va_start(arg, format);
      taglog(level);
      result = vfprintf(stderr, format, arg);
      va_end(arg);
   }
   return result;
}

logger_t new_logger(level_t level) {
   switch(level) {
   case warn:
      return warn_logger;
   case info:
      return info_logger;
   default:
      return debug_logger;
   }
}
