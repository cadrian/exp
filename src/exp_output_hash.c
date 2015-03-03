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
 * This file contains the implementation of "hash", "wordcount", "daemon", and "host" outputs.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <libgen.h>
#include <math.h>
#include <cad_hash.h>

#include "exp_output.h"
#include "exp_filter.h"
#include "exp_fingerprint.h"
#include "exp_file.h"

#define DEFAULT_DICT_CAPACITY 16
#define SAMPLE_THRESHOLD 3

typedef struct output_hash_s output_hash_t;
struct output_hash_s {
     output_t fn;
     const char *type;
     logger_t log;
     input_t *input;
     options_t options;
     filter_t **filters;
     cad_hash_t *dict;
     size_t max_count;
     fingerprint_t *fingerprint;
     size_t meancount;
     size_t devcount;
     void (*fill)(output_hash_t*,input_file_t*,filter_t*);
};

typedef struct {
     size_t count;
     size_t capacity;
     entry_t **entries;
     char key[0];
} dict_entry_t;

typedef struct {
     logger_t log;
     size_t fgcount;
     size_t count;
     size_t delcount;
     output_hash_t *data;
} fingerprint_data_t;

typedef void (*fingerprint_iterator_fn)(const char *key, fingerprint_data_t *data);

static const char *hash_key(entry_t *entry) {
     const char *result;
     static char buffer[MAX_LINE_SIZE];
     const char *daemon, *logline;
     daemon = entry->daemon(entry);
     logline = entry->logline(entry);
     if (logline == NULL) {
          result = "#";
     } else if (daemon == NULL || daemon[0] == '\0') {
          result = logline;
     } else {
          snprintf(buffer, MAX_LINE_SIZE, "%s %s", daemon, logline);
          result = buffer;
     }
     return result;
}

static void fingerprint_iterate(input_file_t *file, filter_t *filter, fingerprint_iterator_fn iterator, fingerprint_data_t *data) {
     /*
      * Quick'n'dirty way to iterate over unique keys of one file
      */
     entry_t *entry;
     int i, n = file->entries_length(file);
     const char *key;
     cad_hash_t *dict = cad_new_hash(stdlib_memory, cad_hash_strings);
     for (i = 0; i < n; i++) {
          entry = file->entry(file, i);
          key = filter->scrub(filter, hash_key(entry));
          if (dict->get(dict, key) == NULL) {
               iterator(key, data);
               dict->set(dict, key, entry);
          }
     }
     dict->free(dict);
}

static void fingerprint_file_count(const char *key, fingerprint_data_t *data) {
     if (data->data->dict->get(data->data->dict, key) != NULL) {
          data->count++;
     }
     data->fgcount++;
}

static void fingerprint_file_del_key(const char *key, fingerprint_data_t *data) {
     if (data->data->dict->del(data->data->dict, key) != NULL) {
          data->delcount++;
     }
}

static int hash_increment(output_hash_t *this, const char *key, entry_t *value) {
     dict_entry_t *entry = this->dict->get(this->dict, key);
     if (entry == NULL) {
          entry = malloc(sizeof(dict_entry_t) + strlen(key) + 1);
          entry->count = 0;
          entry->capacity = DEFAULT_DICT_CAPACITY;
          entry->entries = malloc(entry->capacity * sizeof(entry_t*));
          this->dict->set(this->dict, key, entry);
          strcpy(entry->key, key);
     } else if (entry->count == entry->capacity) {
          entry->capacity *= 2;
          entry->entries = realloc(entry->entries, entry->capacity * sizeof(entry_t*));
     }
     entry->entries[entry->count++] = value;
     if (entry->count > this->max_count) {
          this->max_count = entry->count;
     }
     return entry->count;
}

static void fingerprint_increment(output_hash_t *this, input_file_t *fingerprint_file) {
     /*
      * Called from the fingerprint output, add the fingerprint key
      */
     entry_factory_t *factory;
     entry_t *fingerprint_entry;
     line_t *fingerprint_line;
     char buffer[MAX_LINE_SIZE];
     const char *filename;

     factory = entry_factory_named("raw");
     strcpy(buffer, fingerprint_file->get_name(fingerprint_file));
     filename = basename(buffer);
     this->log(info, "Adding fingerprint: %s\n", filename);
     fingerprint_line = new_line(NULL, strlen(filename), filename);
     fingerprint_entry = factory->new_entry(factory, fingerprint_line);
     hash_increment(this, filename, fingerprint_entry);
}

static bool_t output_hash_fingerprint_file(output_hash_t *this, int index, output_hash_t *data) {
     /*
      * This function runs in the "output" embedded in the fingerprint object.
      * The given "data" is the actual output to filter if a fingerprint is found.
      */
     bool_t result = false;
     fingerprint_data_t fingerprint = {
          .log = this->log,
          .fgcount = 0,
          .count = 0,
          .delcount = 0,
          .data = data,
     };
     input_file_t *file = this->input->file(this->input, index);
     filter_t *filter = this->filters[index];
     size_t threshold;

     fingerprint_iterate(file, filter, fingerprint_file_count, &fingerprint);
     threshold = (int)floor(THRESHOLD_COEFFICIENT * (double)fingerprint.fgcount);
     this->log(debug, "Threshold %s: %lu/%lu\n", file->get_name(file), (unsigned long)threshold, (unsigned long)fingerprint.fgcount);

     if (fingerprint.count > threshold) {
          this->log(debug, "Found fingerprint (%lu > %lu): %s\n", (unsigned long)fingerprint.count, (unsigned long)threshold, file->get_name(file));
          fingerprint_iterate(file, filter, fingerprint_file_del_key, &fingerprint);
          fingerprint_increment(data, file);
          this->log(debug, "Removed %lu keys\n", (unsigned long)fingerprint.delcount);
          result = true;
     } else {
          this->log(debug, "%lu < %lu\n", (unsigned long)fingerprint.count, (unsigned long)threshold);
     }

     return result;
}

static void hash_fill_(output_hash_t *this, input_file_t *file, filter_t *filter) {
     int i, n = file->entries_length(file);
     entry_t *entry;
     const char *key;
     for (i = 0; i < n; i++) {
          entry = file->entry(file, i);
          key = filter->scrub(filter, hash_key(entry));
          hash_increment(this, key, entry);
     }
}

static void wordcount_fill_(output_hash_t *this, input_file_t *file, filter_t *filter) {
     int i, n = file->entries_length(file), inc;
     entry_t *entry;
     char keybuf[MAX_LINE_SIZE];
     char *key, *next;
     bool_t full;
     for (i = 0; i < n; i++) {
          entry = file->entry(file, i);
          this->log(info, "Wordcount %d/%d | %s\n", i+1, n, entry->logline(entry));
          strcpy(keybuf, filter->scrub(filter, entry->logline(entry)));
          key = next = keybuf;
          this->log(debug, "Wordcount fill | %s | %s\n", entry->logline(entry), key);
          full = false;
          while (*next) {
               switch(*next) {
               case ' ':
               case '\t':
                    if (full) {
                         *next = '\0';
                         inc = hash_increment(this, key, entry);
                         this->log(debug, " <%d> %s\n", inc, key);
                         full = false;
                    }
                    key = next + 1;
                    break;
               default:
                    full = true;
                    break;
               }
               next++;
          }
          if (full) {
               inc = hash_increment(this, key, entry);
               this->log(debug, " <%d> %s\n", inc, key);
          }
     }
}

static void daemon_fill_(output_hash_t *this, input_file_t *file, filter_t *filter) {
     int i, n = file->entries_length(file);
     entry_t *entry;
     const char *key;
     for (i = 0; i < n; i++) {
          entry = file->entry(file, i);
          key = filter->scrub(filter, entry->daemon(entry));
          hash_increment(this, key, entry);
     }
}

static void host_fill_(output_hash_t *this, input_file_t *file, filter_t *filter) {
     int i, n = file->entries_length(file), inc;
     entry_t *entry;
     const char *key;
     for (i = 0; i < n; i++) {
          entry = file->entry(file, i);
          key = filter->scrub(filter, entry->host(entry));
          inc = hash_increment(this, key, entry);
          this->log(debug, "Host %d/%d | %s | %s <%d>\n", i+1, n, entry->host(entry), key, inc);
     }
}

static void hash_fill(output_hash_t *this) {
     int i, n = this->input->files_length(this->input);
     input_file_t *file;
     filter_t *filter;
     dict_entry_t *entry;

     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          filter = this->filters[i];
          this->fill(this, file, filter);
     }

     entry = this->dict->del(this->dict, "#");
     if (entry != NULL) {
          free(entry->entries);
          free(entry);
     }
}

static void hash_calculate_stats(cad_hash_t *dict, int index, const char *key, dict_entry_t *value, output_hash_t *this) {
     size_t count = value->count;
     this->meancount += count;
     this->devcount += count * count;
}

static void output_hash_prepare(output_hash_t *this) {
     int i, n = this->input->files_length(this->input);
     input_file_t *file;
     entry_factory_t *factory;
     filter_t *filter;
     char filename[128];

     this->filters = malloc(n * sizeof(filter_t*));
     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          factory = file->get_factory(file);
          this->filters[i] = filter = new_filter(this->log, this->options.filter_extradirs);
          if (this->options.filter) {
               snprintf(filename, 128, "%s.%s.stopwords", this->type, factory->get_name(factory));
               filter->extend(filter, filename, NULL);
               snprintf(filename, 128, "%s.stopwords", this->type);
               filter->extend(filter, filename, "#");
          }
     }

     hash_fill(this);

     if (this->options.fingerprint) {
          this->fingerprint->run(this->fingerprint, this);
     }

     this->dict->iterate(this->dict, (cad_hash_iterator_fn)hash_calculate_stats, this);
}

static int dict_comp(const dict_entry_t **e1, const dict_entry_t **e2) {
     return strcmp((*e1)->key, (*e2)->key);
}

typedef struct {
     size_t count;
     int n;
     dict_entry_t **entries;
} dict_sort_buf ;

static void hash_display_fill_buf(cad_hash_t *dict, int index, const char *key, dict_entry_t *value, dict_sort_buf *buf) {
     if (value->count == buf->count) {
          buf->entries[buf->n++] = value;
     }
}

static void hash_display_count(output_hash_t *this, size_t count) {
     int i, r;
     dict_entry_t **entries = malloc(this->dict->count(this->dict) * sizeof(dict_entry_t*));
     dict_sort_buf buf = { count, 0, entries };
     entry_t *entry;

     this->dict->iterate(this->dict, (cad_hash_iterator_fn)hash_display_fill_buf, &buf);
     if (buf.n > 0) {
          qsort(entries, buf.n, sizeof(dict_entry_t*), (int(*)(const void*,const void*))dict_comp);
          for (i = 0; i < buf.n; i++) {
               switch(this->options.sample) {
               case sample_none:
                    printf("%lu:\t%s\n", (unsigned long)count, entries[i]->key);
                    break;
               case sample_threshold:
                    if (count <= SAMPLE_THRESHOLD) {
                         entry = entries[i]->entries[0];
                         printf("%lu:\t%s\n", (unsigned long)count, entry->logline(entry));
                    } else {
                         printf("%lu:\t%s\n", (unsigned long)count, entries[i]->key);
                    }
                    break;
               case sample_all:
                    r = rand() % count;
                    entry = entries[i]->entries[r];
                    printf("%lu:\t%s\n", (unsigned long)count, entry->logline(entry));
                    break;
               }
          }
     }

     free(entries);
}

static void output_hash_display(output_hash_t *this) {
     size_t i;
     double mean, dev;

     mean = (double)(this->meancount) / (double)(this->dict->count(this->dict));
     dev = sqrt((double)(this->devcount) / (double)(this->dict->count(this->dict)) - mean * mean);

     this->log(info, "Mean: %g\n", mean);
     this->log(info, "Standard deviation: %g\n", dev);
     dev *= this->options.dev;

     switch(this->options.sample) {
     case sample_none:
          this->log(info, "Sample type: none\n");
          break;
     case sample_threshold:
          this->log(info, "Sample type: threshold\n");
          break;
     case sample_all:
          this->log(info, "Sample type: all\n");
          break;
     }

     for (i = this->max_count; i > 0; i--) {
          this->log(debug, "%lu: %g vs %g\n", (unsigned long)i, fabs(i - mean), dev);
          if (dev == 0 || fabs(i - mean) > dev) {
               hash_display_count(this, i);
          } else {
               this->log(debug, "%lu: with std dev %g - %g\n", (unsigned long)i, mean - dev, mean + dev);
          }
     }
}

static options_set_t output_hash_options_set(output_hash_t *this) {
     static options_set_t result = {
          .filter=true,
          .fingerprint=true,
          .tick=false,
          .wide=false,
          .sample=true,
          .filter_extradirs=true,
          .fingerprint_extradirs=true,
          .factory_extradirs=false,
          .year=true,
          .exp_mode=false,
          .dev=true,
     };
     return result;
}

static options_t output_hash_default_options(output_hash_t *this) {
     static options_t result = {
          .filter = true,
          .fingerprint = false,
          .sample = sample_threshold,
          .dev = 0,
     };
     time_t tm;
     static bool_t init = false;
     if (!init) {
          init = true;
          tm = time(NULL);
          result.year = localtime(&tm)->tm_year + 1900;
     }
     return result;
}

static options_t output_nothash_default_options(output_hash_t *this) {
     static options_t result = {
          .filter = true,
          .fingerprint = false,
          .sample = sample_none,
     };
     return result;
}

static void output_hash_set_options(output_hash_t *this, options_t options) {
     this->options = options;
     if (options.fingerprint) {
          this->fingerprint = new_fingerprint(this->log, options);
     }
}

static output_t output_hash_fn = {
     .fingerprint_file = (output_fingerprint_file_fn)output_hash_fingerprint_file,
     .options_set = (output_options_set_fn)output_hash_options_set,
     .default_options = (output_default_options_fn)output_hash_default_options,
     .set_options = (output_set_options_fn)output_hash_set_options,
     .prepare = (output_prepare_fn)output_hash_prepare,
     .display = (output_display_fn)output_hash_display,
};

static output_t *new_output_(logger_t log, input_t *input, const char *type, void (*fill)(output_hash_t*,input_file_t*,filter_t*)) {
     output_hash_t *result = malloc(sizeof(output_hash_t));
     result->fn = output_hash_fn;
     result->type = type;
     result->log = log;
     result->input = input;
     result->dict = cad_new_hash(stdlib_memory, cad_hash_strings);
     result->max_count = result->meancount = result->devcount = 0;
     result->fill = fill;
     result->fingerprint = NULL;
     memset(&(result->options), 0, sizeof(options_t));
     return &(result->fn);
}

output_t *new_output_hash(logger_t log, input_t *input) {
     return new_output_(log, input, "hash", hash_fill_);
}

output_t *new_output_wordcount(logger_t log, input_t *input) {
     output_t *result = new_output_(log, input, "words", wordcount_fill_);
     result->default_options = (output_default_options_fn)output_nothash_default_options;
     return result;
}

output_t *new_output_daemon(logger_t log, input_t *input) {
     output_t *result = new_output_(log, input, "daemon", daemon_fill_);
     result->default_options = (output_default_options_fn)output_nothash_default_options;
     return result;
}

output_t *new_output_host(logger_t log, input_t *input) {
     output_t *result = new_output_(log, input, "host", host_fill_);
     result->default_options = (output_default_options_fn)output_nothash_default_options;
     return result;
}
