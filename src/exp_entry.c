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

#include "exp_entry.h"
#include "exp_log.h"

#include "entry/_exp_entry_log.c"
#include "entry/_exp_entry_syslog.c"
//#include "entry/_exp_entry_rsyslog.c"
//#include "entry/_exp_entry_apache_access.c"
//#include "entry/_exp_entry_apache_error.c"
//#include "entry/_exp_entry_securelog.c"
//#include "entry/_exp_entry_scriptlog.c"
//#include "entry/_exp_entry_raw.c"
//#include "entry/_exp_entry_snort.c"

entry_factory_t *entry_factories[] = {
     &syslog_entry_factory,
     NULL
};

size_t entry_factories_length(void) {
     static size_t result = 0;
     if (result == 0) {
          while (entry_factories[result] != NULL) {
               result++;
          }
     }
     return result;
}
