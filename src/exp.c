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
#include "exp_input.h"
#include "exp_output.h"

static level_t   verbose     = warn;
static expmode_t mode        = mode_undefined;

static sample_t  sample      = sample_none;

static output_options_t options = {
     .filter = true,
     .fingerprint = false,
     .tick = "#",
     .wide = false,
};

static struct option long_options[] = {
   {"verbose", no_argument, NULL, 'v'},

   {"sample", no_argument, (int*)&sample, sample_threshold},
   {"nosample", no_argument, (int*)&sample, sample_none},
   {"allsample", no_argument, (int*)&sample, sample_all},

   {"filter", no_argument, (int*)&options.filter, true},
   {"nofilter", no_argument, (int*)&options.filter, false},

   {"wide", no_argument, (int*)&options.wide, true},
   {"tick", required_argument, NULL, 't'},
   {"fingerprint", no_argument, (int*)&options.fingerprint, true},

   {"version", no_argument, NULL, 'V'},

   {"hash",      no_argument, NULL, 'x'},
   {"wordcount", no_argument, NULL, 'w'},
   {"daemon",    no_argument, NULL, 'D'},
   {"host",      no_argument, NULL, 'H'},
   {"sgraph",    no_argument, NULL, 's'},
   {"mgraph",    no_argument, NULL, 'm'},
   {"hgraph",    no_argument, NULL, 'h'},
   {"dgraph",    no_argument, NULL, 'd'},
   {"mograph",   no_argument, NULL, 'M'},
   {"ygraph",    no_argument, NULL, 'y'},

   {0,0,0,0}
};

/**
 * Returns the first non-option index
 */
static void parse_options(int argc, char **argv) {
   int option_index = 0;
   int c;
   bool_t done = false;

   while (!done) {
      c = getopt_long(argc, argv, "vt:VxwDHsmhdMy", long_options, &option_index);
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
         options.tick = malloc(strlen(optarg) + 1);
         strcpy(options.tick, optarg);
         break;

      case 'V':
         printf("exp version %d.%d\n", EXP_VERSION_MAJOR, EXP_VERSION_MINOR);
         exit(0);

      case '?':
         /* help handled by getopt */
         break;

      case 'x': mode = mode_hash;      break;
      case 'w': mode = mode_wordcount; break;
      case 'D': mode = mode_daemon;    break;
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
}

int main(int argc, char **argv) {
   logger_t log;
   input_t *input;
   output_t *output;

   parse_options(argc, argv);
   log = new_logger(verbose);

   input = new_input(log);
   switch(mode) {
   case mode_hash:
      output = new_output_hash(log, input, options);
      break;
   case mode_wordcount:
      output = new_output_wordcount(log, input, options);
      break;
   case mode_daemon:
      output = new_output_daemon(log, input, options);
      break;
   case mode_host:
      output = new_output_host(log, input, options);
      break;
   case mode_sgraph:
      output = new_output_sgraph(log, input, options);
      break;
   case mode_mgraph:
      output = new_output_mgraph(log, input, options);
      break;
   case mode_hgraph:
      output = new_output_hgraph(log, input, options);
      break;
   case mode_dgraph:
      output = new_output_dgraph(log, input, options);
      break;
   case mode_mograph:
      output = new_output_mograph(log, input, options);
      break;
   case mode_ygraph:
      output = new_output_ygraph(log, input, options);
      break;
   default:
      fprintf(stderr, "Undefined mode!\n");
      exit(2);
   }

   while (optind < argc) {
      log(debug, "more input %d/%d %s\n", optind, argc, argv[optind]);
      input->parse(input, argv[optind++]);
   }
   log(debug, "input done\n");

   output->display(output);

   return 0;
}
