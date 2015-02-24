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

/**
 * @ingroup exp
 * @file
 *
 * This file contains the main source (entry point of the application).
 */

#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "exp.h"
#include "exp_log.h"
#include "exp_input.h"
#include "exp_output.h"
#include "exp_entry_factory.h"

static level_t   verbose         = warn;
static expmode_t mode            = mode_undefined;

typedef struct {
     int capacity;
     int count;
     char **dirs;
} dirs_t;

static dirs_t filterdirs = {0,0,NULL};
static dirs_t fingerprintdirs = {0,0,NULL};

/**
 * The options setting mask.
 */
static options_set_t options_set = {
     false, false, false, false, false, false, false,
};

/**
 * The actual options.
 */
static output_options_t options = {
     .filter = true,
     .fingerprint = false,
     .tick = "#",
     .wide = false,
     .sample = sample_threshold,
     .filter_extradirs= NULL,
     .fingerprint_extradirs= NULL,
     .year = 0,
     .exp_mode = false,
};

/**
 * The usage.
 *
 * @param[in] cmd the command name.
 */
static void usage(const char *cmd) {
     fprintf(stderr,
             "Usage: %s [options] [file...]\n\nOptions:\n"
             "  -h, --help         Show this help message and exit\n"
             "  -v, --verbose      Show verbose output (may be specified twice)\n"
             "  -V, --version      Show verbose output\n"
             "\n"
             "  -x, --hash         Show hashes of log files with numbers removed\n"
             "  -w, --wordcount    Show word count for given word\n"
             "  -D, --daemon       Show a report of entries from each daemon\n"
             "  -H, --host         Show a report of entries from each host\n"
             "  -s, --sgraph       Show a graph of the first 60 seconds\n"
             "  -m, --mgraph       Show a graph of the first 60 minutes\n"
             "  -X, --hgraph       Show a graph of the first 24 hours\n"
             "  -d, --dgraph       Show a graph of the first 31 days\n"
             "  -M, --mograph      Show a graph of the first 12 months\n"
             "  -y, --ygraph       Show a graph of the first 10 years\n"
             "\n"
             "  hash-specific options:\n"
             "  --sample           Show sample output for small numbered entries\n"
             "  --nosample         Do not sample output for low count entries\n"
             "  --allsample        Show samples instead of munged text for all entries\n"
             "  --filter           Use filter files during processing\n"
             "  --nofilter         Do not use filter files during processing\n"
             "  --fingerprint      Use fingerprinting to remove certain patterns\n"
             "\n"
             "  graph-specific options:\n"
             "  --wide             Use wider graph characters\n"
             "  -t, --tick=TICK    Change tick character from default\n"
             "\n"
             "If no file is specified, data is read from stdin.\n\n", cmd);
}

/**
 * Print ExP's version.
 */
static void print_version(void) {
     printf("ExP version %d.%d.%d\nCopyleft (C) 2015, Cyril Adrian <cyril.adrian@gmail.com>\n", EXP_GRAND_VERSION, EXP_MAJOR_VERSION, EXP_MINOR_VERSION);
}

/**
 * The long options definition.
 */
static struct option long_options[] = {
     {"verbose",        no_argument,       NULL, 'v'},
     {"help",           no_argument,       NULL, 'h'},
     {"version",        no_argument,       NULL, 'V'},

     {"sample",         no_argument,       NULL,  1 },
     {"nosample",       no_argument,       NULL,  2 },
     {"allsample",      no_argument,       NULL,  3 },

     {"filter",         no_argument,       NULL,  4 },
     {"nofilter",       no_argument,       NULL,  5 },

     {"wide",           no_argument,       NULL,  6 },
     {"fingerprint",    no_argument,       NULL,  7 },
     {"tick",           required_argument, NULL, 't'},

     {"hash",           no_argument,       NULL, 'x'},
     {"wordcount",      no_argument,       NULL, 'w'},
     {"daemon",         no_argument,       NULL, 'D'},
     {"host",           no_argument,       NULL, 'H'},
     {"sgraph",         no_argument,       NULL, 's'},
     {"mgraph",         no_argument,       NULL, 'm'},
     {"hgraph",         no_argument,       NULL, 'X'},
     {"dgraph",         no_argument,       NULL, 'd'},
     {"mograph",        no_argument,       NULL, 'M'},
     {"ygraph",         no_argument,       NULL, 'y'},

     {"filterdir",      required_argument, NULL,  8 },
     {"fingerprintdir", required_argument, NULL,  9 },
     {"year",           required_argument, NULL, 10 },
     {"exp_mode",       no_argument,       NULL, 11 },

     {0,0,0,0}
};

/**
 * Set the mode, does not allow to set twice
 *
 * @param[in] m the mode to set
 */
static void set_mode(int m) {
     if (mode != mode_undefined) {
          fprintf(stderr, "**** Error: cannot set mode twice\n");
          exit(2);
     }
     mode = m;
}

/**
 * Add an extra directory, ensures that the array always finishes with a NULL entry
 *
 * @param[in] dirs the directory array
 * @param[in] dir the directory to add
 */
static void add_extradir(dirs_t *dirs, const char *dir) {
     int size;
     if (dirs->capacity == dirs->count) {
          if (dirs->capacity == 0) {
               dirs->capacity = 2;
               size = (dirs->capacity + 1) * sizeof(char*);
               dirs->dirs = malloc(size);
               memset(dirs->dirs, 0, size);
          } else {
               dirs->capacity *= 2;
               size = (dirs->capacity + 1) * sizeof(char*);
               dirs->dirs = realloc(dirs->dirs, size);
               memset(dirs->dirs + dirs->count, 0, size - (dirs->count) * sizeof(char*));
          }
          dirs->dirs[dirs->count++] = strdup(dir);
     }
}

static void parse_options(int argc, char * const argv[]) {
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

          case 1: // sample
               options_set.sample = true;
               options.sample = sample_threshold;
               break;
          case 2: // nosample
               options_set.sample = true;
               options.sample = sample_none;
               break;
          case 3: // allsample
               options_set.sample = true;
               options.sample = sample_all;
               break;

          case 4: // filter
               options_set.filter = true;
               options.filter = true;
               break;
          case 5: // nofilter
               options_set.filter = true;
               options.filter = false;
               break;

          case 6: // wide
               options_set.wide = true;
               options.wide = true;
               break;

          case 7: // fingerprint
               options_set.fingerprint = true;
               options.fingerprint = true;
               break;

          case 'v':
               verbose++;
               break;

          case 't':
               options_set.tick = true;
               options.tick = malloc(strlen(optarg) + 1);
               strcpy(options.tick, optarg);
               break;

          case 'V':
               print_version();
               exit(0);

          case 'h':
               usage(argv[0]);
               exit(0);
               break;

          case 'x': set_mode(mode_hash);      break;
          case 'w': set_mode(mode_wordcount); break;
          case 'D': set_mode(mode_daemon);    break;
          case 'H': set_mode(mode_host);      break;
          case 's': set_mode(mode_sgraph);    break;
          case 'm': set_mode(mode_mgraph);    break;
          case 'X': set_mode(mode_hgraph);    break;
          case 'd': set_mode(mode_dgraph);    break;
          case 'M': set_mode(mode_mograph);   break;
          case 'y': set_mode(mode_ygraph);    break;

          case 8:
               add_extradir(&filterdirs, optarg);
               options_set.filter_extradirs = true;
               break;

          case 9:
               add_extradir(&fingerprintdirs, optarg);
               options_set.fingerprint_extradirs = true;
               break;

          case 10:
               options.year = atoi(optarg);
               options_set.year = true;
               break;

          case 11:
               options.exp_mode = true;
               options_set.exp_mode = true;
               break;

          case '?':
          default:
               usage(argv[0]);
               exit(2);
          }
     }
}

#define check_option(option) do {                                       \
     if (allowed_options.option) {                                      \
          if (!options_set.option) {                                    \
               options.option = default_options.option;                 \
               log(debug, "Setting default option: %s\n", #option);     \
          }                                                             \
     } else if (options_set.option) {                                   \
          log(warn, "Incompatible option: %s (ignored)\n", #option);    \
     }                                                                  \
} while(0)

static void check_options_set(logger_t log, options_set_t allowed_options, output_options_t default_options) {
     check_option(filter);
     check_option(fingerprint);
     check_option(tick);
     check_option(wide);
     check_option(sample);
     check_option(filter_extradirs);
     check_option(fingerprint_extradirs);
     check_option(year);
}

/**
 * the application entry point.
 *
 * @param[in] argc the number of arguments on the command line
 * @param[in] argv the arguments on the command line
 */
int main(int argc, char * const argv[]) {
     logger_t log;
     input_t *input;
     output_t *output;
     int lastoptind;
     bool_t has_data = false;

     srand(time(NULL));

     parse_options(argc, argv);
     log = new_logger(verbose);
     input = new_input(log);

     switch(mode) {
     case mode_hash:
          output = new_output_hash(log, input);
          break;
     case mode_wordcount:
          output = new_output_wordcount(log, input);
          break;
     case mode_daemon:
          output = new_output_daemon(log, input);
          break;
     case mode_host:
          output = new_output_host(log, input);
          break;
     case mode_sgraph:
          output = new_output_sgraph(log, input);
          break;
     case mode_mgraph:
          output = new_output_mgraph(log, input);
          break;
     case mode_hgraph:
          output = new_output_hgraph(log, input);
          break;
     case mode_dgraph:
          output = new_output_dgraph(log, input);
          break;
     case mode_mograph:
          output = new_output_mograph(log, input);
          break;
     case mode_ygraph:
          output = new_output_ygraph(log, input);
          break;
     default:
          if (argc == optind) {
               print_version();
               exit(0);
          }
          fprintf(stderr, "**** Undefined mode\n");
          exit(2);
     }

     register_all_factories(log);
     sort_factories(log);

     check_options_set(log, output->options_set(output), output->default_options(output));
     options.filter_extradirs = filterdirs.dirs;
     options.fingerprint_extradirs = fingerprintdirs.dirs;
     output->set_options(output, options);

     if (optind == argc) {
          log(debug, "Input: stdin\n");
          if (input->parse(input, "-") != NULL) {
               has_data = true;
          }
     } else{
          lastoptind = optind;
          while (optind < argc) {
               log(debug, "Input %d/%d: %s\n", optind - lastoptind + 1, argc - lastoptind, argv[optind]);
               if (input->parse(input, argv[optind++]) != NULL) {
                    has_data = true;
               }
          }
     }
     if (has_data) {
          input->sort_files(input);
          log(debug, "Input done\n");
          output->prepare(output);
          output->display(output);
     } else {
          fputs("No data found\n", stdout);
     }

     return 0;
}
