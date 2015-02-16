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

#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>

#include "exp.h"
#include "exp_log.h"

static level_t   verbose     = warn;
static sample_t  sample      = sample_none;
static bool_t    filter      = true;
static bool_t    wide        = false;
static expmode_t mode        = mode_undefined;
static char*     tick        = "#";
static bool_t    fingerprint = false;

/**
 * Returns the first non-option index
 */
static int parse_options(int argc, char **argv) {
   static struct option long_options[] = {
      {"verbose", no_argument, NULL, 'v'},

      {"sample", no_argument, (int*)&sample, sample_threshold},
      {"nosample", no_argument, (int*)&sample, sample_none},
      {"allsample", no_argument, (int*)&sample, sample_all},

      {"filter", no_argument, (int*)&filter, true},
      {"nofilter", no_argument, (int*)&filter, false},

      {"wide", no_argument, (int*)&wide, true},
      {"tick", required_argument, NULL, 't'},
      {"fingerprint", no_argument, (int*)&fingerprint, true},

      {"version", no_argument, NULL, 'V'},

      {"hash",      no_argument, NULL, 'x'},
      {"wordcount", no_argument, NULL, 'w'},
      {"host",      no_argument, NULL, 'H'},
      {"sgraph",    no_argument, NULL, 's'},
      {"mgraph",    no_argument, NULL, 'm'},
      {"hgraph",    no_argument, NULL, 'h'},
      {"dgraph",    no_argument, NULL, 'd'},
      {"mograph",   no_argument, NULL, 'M'},
      {"ygraph",    no_argument, NULL, 'y'},

      {0,0,0,0}
   };

   int option_index = 0;
   int c;
   bool_t done = false;

   while (!done) {
      c = getopt_long(argc, argv, "vt:VxwHsmhdMy", long_options, &option_index);
      switch(c) {
      case 0:
         /* flag was set */
         break;

      case -1:
         done = true;
         break;

      case 'v':
         verbose++;
         break;

      case 't':
         tick = malloc(strlen(optarg) + 1);
         strcpy(tick, optarg);
         break;

      case 'V':
         printf("exp version %d.%d\n", EXP_VERSION_MAJOR, EXP_VERSION_MINOR);
         exit(0);

      case '?':
         /* help handled by getopt */
         break;

      case 'x': mode = mode_hash;      break;
      case 'w': mode = mode_wordcount; break;
      case 'H': mode = mode_host;      break;
      case 's': mode = mode_sgraph;    break;
      case 'm': mode = mode_mgraph;    break;
      case 'h': mode = mode_hgraph;    break;
      case 'd': mode = mode_dgraph;    break;
      case 'M': mode = mode_mograph;   break;
      case 'y': mode = mode_ygraph;    break;

      default:
         abort();
      }
   }

   return optind;
}

int main(int argc, char **argv) {
   logger_t log;
   int argindex = parse_options(argc, argv);
   log = new_logger(verbose);

   log(warn, "warning\n");
   log(info, "info\n");
   log(debug, "debug\n");

   return 0;
}
