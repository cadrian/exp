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
 * @ingroup exp_output
 * @file
 *
 * This file contains the implementation of fingerprints.
 */

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "exp_fingerprint.h"
#include "exp_log.h"
#include "exp_input.h"
#include "exp_output.h"

static const char *dirs[] = {
     "/var/lib/exp/fingerprints/",
     "/usr/local/exp/var/lib/fingerprints/",
     "/opt/exp/var/lib/fingerprints/",
     "/var/lib/petit/fingerprints/",
     "/usr/local/petit/var/lib/fingerprints/",
     "/opt/petit/var/lib/fingerprints/",
     NULL
};

typedef struct fingerprint_impl_s fingerprint_impl_t;

struct fingerprint_impl_s {
     fingerprint_t fn;
     logger_t log;
     input_t *input;
     output_t *output;
};

static void fingerprint_impl_run(fingerprint_impl_t *this, void *output) {
     int i, n = this->input->files_length(this->input);
     bool_t done = false;
     for (i = 0; !done && i < n; i++) {
          done = this->output->fingerprint_file(this->output, i, output);
     }
}

static fingerprint_t fingerprint_impl_fn = {
     .run = (fingerprint_run_fn)fingerprint_impl_run,
};

/**
 * Select only ".ep" files
 */
static int fingerprint_file_selector(const struct dirent *ep) {
     int result = 0;
     char *dot = strrchr(ep->d_name, '.');
     if (dot != NULL) {
          result = strcmp(dot, ".fp") == 0;
     }
     return result;
}

static void parse_entry(fingerprint_impl_t *this, const char *dir, const char *filename) {
     char *path = malloc(strlen(dir) + strlen(filename) + 1);
     sprintf(path, "%s%s", dir, filename);
     this->input->parse(this->input, path);
     free(path);
}

static void scan_input_dir(fingerprint_impl_t *this, const char *dir) {
     struct dirent **namelist;
     int count = scandir(dir, &namelist, fingerprint_file_selector, alphasort);
     int i;

     if (count < 0) {
          this->log(debug, "Error opening directory %s: %s\n", dir, strerror(errno));
     } else {
          for (i = 0; i < count; i++) {
               parse_entry(this, dir, namelist[i]->d_name);
               free(namelist[i]);
          }
          free(namelist);
     }
}

static void prepare_input(fingerprint_impl_t *this) {
     const char *dir;
     int i;
     for (i = 0; (dir = dirs[i]) != NULL; i++) {
          scan_input_dir(this, dir);
     }
     this->input->sort_files(this->input);
}

fingerprint_t *new_fingerprint(logger_t log) {
     output_options_t options = {
          .filter = true,
          .fingerprint = false,
          .sample = sample_none,
     };
     fingerprint_impl_t *result = malloc(sizeof(fingerprint_impl_t));
     result->fn = fingerprint_impl_fn;
     result->log = log;
     result->input = new_input(log);
     prepare_input(result);
     result->output = new_output_hash(log, result->input, options);
     return &(result->fn);
}
