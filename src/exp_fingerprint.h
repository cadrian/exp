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

#ifndef __EXP_FINGERPRINT_H__
#define __EXP_FINGERPRINT_H__

#include "exp.h"
#include "exp_log.h"

#define THRESHOLD_COEFFICIENT 0.31

typedef struct fingerprint_s fingerprint_t;

typedef void (*fingerprint_run_fn)(fingerprint_t *this, void *output);

struct fingerprint_s {
     fingerprint_run_fn run;
};

fingerprint_t *new_fingerprint(logger_t log);

#endif /* __EXP_FINGERPRINT_H__ */
