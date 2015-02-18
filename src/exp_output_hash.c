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
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "exp_output.h"
#include "exp_filter.h"

#include <cad_hash.h>

#define DEFAULT_DICT_CAPACITY 16
#define SAMPLE_THRESHOLD 3

typedef struct {
     output_t fn;
     logger_t log;
     input_t *input;
     output_options_t options;
     filter_t **filters;
     cad_hash_t *dict;
     size_t max_count;
} output_hash_t;

typedef struct {
     size_t count;
     size_t capacity;
     entry_t **entries;
     char key[0];
} dict_entry_t;

static void hash_increment(output_hash_t *this, const char *key, entry_t *value) {
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
}

static void hash_fill_(output_hash_t *this, input_file_t *file, filter_t *filter) {
     int i, n = file->entries_length(file);
     entry_t *entry;
     static char input[MAX_LINE_SIZE];
     const char *key, *daemon, *logline;
     for (i = 0; i < n; i++) {
          entry = file->entry(file, i);
          daemon = entry->daemon(entry);
          logline = entry->logline(entry);
          if (daemon == NULL) {
               key = filter->scrub(filter, logline);
          } else {
               snprintf(input, MAX_LINE_SIZE, "%s %s", daemon, logline);
               key = filter->scrub(filter, input);
          }
          hash_increment(this, key, entry);
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
          hash_fill_(this, file, filter);
     }

     entry = this->dict->del(this->dict, "#");
     if (entry != NULL) {
          free(entry->entries);
          free(entry);
     }
}

static void hash_prepare(output_hash_t *this) {
     int i, n = this->input->files_length(this->input);
     input_file_t *file;
     entry_factory_t *factory;
     filter_t *filter;
     char filename[128];

     this->filters = malloc(n * sizeof(filter_t*));
     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          factory = file->get_factory(file);
          this->filters[i] = filter = new_filter(this->log);
          if (this->options.filter) {
               filter->extend(filter, "hash.stopwords", "#");
               snprintf(filename, 128, "hash.%s.stopwords", factory->get_name(factory));
               filter->extend(filter, filename, NULL);
          }
     }

     if (this->options.fingerprint) {
          this->log(warn, "fingerprint not yet supported\n");
     }

     hash_fill(this);
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
     int i;
     dict_entry_t **entries = malloc(this->dict->count(this->dict) * sizeof(dict_entry_t*));
     dict_sort_buf buf = { count, 0, entries };
     entry_t *entry;

     this->dict->iterate(this->dict, (cad_hash_iterator_fn)hash_display_fill_buf, &buf);
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
               entry = entries[i]->entries[(int)(rand() % count + 1)];
               printf("%lu:        %s\n", (unsigned long)count, entry->logline(entry));
               break;
          }
     }

     free(entries);
}

static void hash_display(output_hash_t *this) {
     size_t i;

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

     srand(time(NULL));
     for (i = this->max_count; i > 0; i--) {
          hash_display_count(this, i);
     }
}

static void output_hash_display(output_hash_t *this) {
     hash_prepare(this);
     hash_display(this);
}

static output_t output_hash_fn = {
   .display = (output_display_fn)output_hash_display,
};

output_t *new_output_hash(logger_t log, input_t *input, output_options_t options) {
   output_hash_t *result = malloc(sizeof(output_hash_t));
   result->fn = output_hash_fn;
   result->log = log;
   result->input = input;
   result->options = options;
   result->dict = cad_new_hash(stdlib_memory, cad_hash_strings);
   result->max_count = 0;
   return &(result->fn);
}
