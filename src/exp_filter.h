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

#ifndef __EXP_FILTER_H__
#define __EXP_FILTER_H__

#include "exp.h"
#include "exp_log.h"

typedef struct filter_s filter_t;

typedef void (*filter_extend_fn)(filter_t *this, const char *filename, const char *replacement);
typedef const char *(*filter_scrub_fn)(filter_t *this, const char *line);
typedef bool_t (*filter_bleach_fn)(filter_t *this, const char *line);

struct filter_s {
     filter_extend_fn extend;
     filter_scrub_fn scrub;
     filter_bleach_fn bleach;
};

filter_t *new_filter(logger_t log);

#endif /* __EXP_FILTER_H__ */
