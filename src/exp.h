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

#ifndef __EXP_H__
#define __EXP_H__

#include "cad_shared.h"

#define EXP_VERSION_MAJOR 0
#define EXP_VERSION_MINOR 1

/**
 * The boolean type.
 */
typedef enum {
   false=0,
   true
} bool_t;

typedef enum {
   sample_none=0,
   sample_threshold=1,
   sample_all=2
} sample_t;

typedef enum {
   mode_undefined=0,
   mode_hash,
   mode_wordcount,
   mode_daemon,
   mode_host,
   mode_sgraph,
   mode_mgraph,
   mode_hgraph,
   mode_dgraph,
   mode_mograph,
   mode_ygraph
} expmode_t;

int month_of(const char *month);

#endif /* __EXP_H__ */
