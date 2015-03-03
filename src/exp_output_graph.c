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
 * This file contains the implementation of "graph" outputs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <cad_hash.h>

#include "exp_output.h"
#include "exp_file.h"

#define DEFAULT_GRAPH_HEIGHT 6

typedef struct output_graph_s output_graph_t;
typedef void (*time_fn)(output_graph_t *this, struct tm *time, entry_t *entry);
typedef void (*increment_time_fn)(struct tm *time);
typedef int (*value_fn)(struct tm *time);

struct output_graph_s {
     output_t fn;
     logger_t log;
     input_t *input;
     options_t options;
     const char *unit;
     int duration;
     time_fn time;
     increment_time_fn increment_time;
     value_fn value;
     cad_hash_t *dict;
     double minh, minz, max;
     struct tm start, middle, end;
     char **keys;
};

static const char* strdate(struct tm*time) {
     static char result[128];
     strftime(result, 128, "%Y-%m-%d %H:%M:%S ", time);
     return result;
}

static void graph_prepare_max(cad_hash_t *hash, int index, const char *key, double *value, output_graph_t *graph) {
     if (graph->max < *value) {
          graph->max = *value;
     }
}

static void graph_prepare_minz(cad_hash_t *hash, int index, const char *key, double *value, output_graph_t *graph) {
     if (graph->minz > *value) {
          graph->minz = *value;
     }
}

static void graph_prepare_minh(cad_hash_t *hash, int index, const char *key, double *value, output_graph_t *graph) {
     if (*value > 0 && graph->minh >= *value) {
          graph->minh = *value / 2;
     }
}

static int cmpstr(const char **a, const char **b) {
     return strcmp(*a, *b);
}

static void output_graph_prepare(output_graph_t *this) {
     struct tm current = {0,};
     bool_t is_first = true;
     input_file_t *file;
     entry_t *entry;
     int i, n = this->input->files_length(this->input), j, m;
     const char *key;
     double *value;

     this->start = this->middle = this->end = current;

     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          if (file->entries_length(file) > 0) {
               entry = file->entry(file, 0);
               this->time(this, &current, entry);
               if (is_first || (difftime(mktime(&current), mktime(&this->start)) < 0)) {
                    this->start = current;
               }
               is_first = false;
          }
     }

     current = this->start;
     for (i = 0; i < this->duration; i++) {
          if (i > 0) {
               this->increment_time(&current);
          }
          if (i == this->duration / 2) {
               this->middle = current;
          }
          key = strdate(&current);
          value = malloc(sizeof(double));
          *value = 0.0;
          this->dict->set(this->dict, key, value);
          this->keys[i] = strdup(key);
     }
     this->end = current;
     qsort(this->keys, this->duration, sizeof(char*), (int(*)(const void*, const void*))cmpstr);

     this->log(debug, "Graph keys:\n");
     for (i = 0; i < this->duration; i++) {
          this->log(debug, "%4d\t%s\n", i+1, this->keys[i]);
     }

     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          m = file->entries_length(file);
          for (j = 0; j < m;j++) {
               entry = file->entry(file, j);
               this->time(this, &current, entry);
               key = strdate(&current);
               value = this->dict->get(this->dict, key);
               if (value != NULL) {
                    *value = (*value) + 1;
               }
          }
     }
     this->dict->iterate(this->dict, (cad_hash_iterator_fn)graph_prepare_max, this);
     this->minz = this->minh = this->max;
     this->dict->iterate(this->dict, (cad_hash_iterator_fn)graph_prepare_minz, this);
     if (this->minz != 0) {
          this->minh = this->minz;
     } else {
          this->dict->iterate(this->dict, (cad_hash_iterator_fn)graph_prepare_minh, this);
     }
}

typedef struct {
     logger_t log;
     int height;
     int width;
     double min;
     double max;
} graph_t;

static void graph_normalize(cad_hash_t *hash, int index, const char *key, double *value, graph_t *graph) {
     double normalized;
     if (*value >= 0) {
          if (graph->max > graph->min) {
               normalized = ceil(graph->height* (*value - graph->min) / (graph->max - graph->min));
               graph->log(debug, "ceil(((%.4g - %.4g) * %d) / (%.4g - %.4g)) = %.4g\n", *value, graph->min, graph->height, graph->max, graph->min, normalized);
          } else {
               normalized = ceil(graph->height * (*value) / (graph->max));
               graph->log(debug, "ceil(%.4g * %d / %.4g) = %.4g\n", *value, graph->height, graph->max, normalized);
          }
          *value = normalized;
     }
}

static void output_graph_display(output_graph_t *this) {
     graph_t graph = {
          .log = this->log,
          .height = DEFAULT_GRAPH_HEIGHT,
          .width = this->duration,
          .min = floor(this->minh),
          .max = floor(this->max),
     };
     double scale;
     char char_fill[4];
     char char_blank[4];
     char char_scale[64];
     int y, i, p;

     int graph_position_start, graph_position_middle, graph_position_end;
     int graph_width;
     const char *key;
     double *value;

     scale = (this->max - this->minz) / graph.height;

     this->log(info, "Graph: dates from %sto %s-- occurrences between %.4g and %.4g\n", strdate(&(this->start)), strdate(&(this->end)), graph.min, graph.max);

     for (i = 0; i < this->duration; i++) {
          key = this->keys[i];
          value = this->dict->get(this->dict, key);
          this->log(debug, "Count <%.4g> | %s\n", *value, key);
     }

     this->dict->iterate(this->dict, (cad_hash_iterator_fn)graph_normalize, &graph);

     for (i = 0; i < this->duration; i++) {
          key = this->keys[i];
          value = this->dict->get(this->dict, key);
          this->log(debug, "Normalized <%.4g> | %s\n", *value, key);
     }

     if (this->options.wide) {
          sprintf(char_fill, "%c%c", this->options.tick[0], this->options.tick[1] == '\0' ? ' ' : this->options.tick[1]);
          strcpy(char_blank, "  ");
          graph_width = graph.width * 2;
          graph_position_start = 1;
          graph_position_middle = graph.width - ((int)(graph.width) % 2);
          graph_position_end = graph_width - 3;
     } else {
          sprintf(char_fill, "%c", this->options.tick[0]);
          strcpy(char_blank, " ");
          graph_width = graph.width;
          graph_position_start = 1;
          graph_position_middle = graph_width / 2;
          graph_position_end = graph_width - 2;
     }
     this->log(debug, "wide: %s fill='%s' blank='%s'\n", this->options.wide ? "true":"false", char_fill, char_blank);

     if (this->options.exp_mode) {
          p = (int)ceil(log10(graph.max)) + 1;
     }

     fputc('\n', stdout);
     for (y = graph.height; y > 0; y--) {
          if (this->options.exp_mode) {
               if (y == graph.height) {
                    sprintf(char_scale, "% *d", p, (int)graph.max);
               } else if (y == 1) {
                    sprintf(char_scale, "% *d", p, (int)graph.min);
               } else {
                    char_scale[0] = '\0';
               }
               printf(" %*.*s - ", p+1, p+1, char_scale);
          }
          for (i = 0; i < this->duration; i++) {
               key = this->keys[i];
               value = this->dict->get(this->dict, key);
               if (value != NULL) {
                    if (*value >= y) {
                         fputs(char_fill, stdout);
                    } else {
                         fputs(char_blank, stdout);
                    }
               }
          }
          fputc('\n', stdout);
     }

     if (this->options.exp_mode) {
          printf(" %*s   ", p+1, "");
     }
     for (i = 0; i < this->duration; i++) {
          fputs(char_fill, stdout);
     }
     fputc('\n', stdout);

     if (this->options.exp_mode) {
          printf(" %*s   ", p+1, "");
     }
     for (i = 1; i < graph_width; i++) {
          if (i == graph_position_start) {
               printf("%.2d", this->value(&(this->start)) % 100);
          } else if (i == graph_position_middle) {
               printf("%.2d", this->value(&(this->middle)) % 100);
          } else if (i == graph_position_end) {
               printf("%.2d", this->value(&(this->end)) % 100);
          } else {
               fputc(' ', stdout);
          }
     }
     fputc('\n', stdout);

     fputc('\n', stdout);
     printf("Start Time:\t%s\t\tMinimum Value: %g\n", strdate(&(this->start)), this->minz);
     printf("End Time:\t%s\t\tMaximum Value: %g\n", strdate(&(this->end)), this->max);
     printf("Duration:\t%d %ss \t\t\tScale: %.12g%s\n", this->duration, this->unit, scale, (scale == (int)scale) ? ".0" : "");
     fputc('\n', stdout);
}

static options_set_t output_graph_options_set(output_graph_t *this) {
     static options_set_t result = {
          .filter=false,
          .fingerprint=false,
          .tick=true,
          .wide=true,
          .sample=false,
          .filter_extradirs=false,
          .fingerprint_extradirs=false,
          .factory_extradirs=false,
          .year=true,
          .exp_mode=true,
          .dev=false,
     };
     return result;
}

static options_t output_graph_default_options(output_graph_t *this) {
     static options_t result = {
          .tick = "#",
          .wide = false,
          .exp_mode = false,
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

static void output_graph_set_options(output_graph_t *this, options_t options) {
     this->options = options;
}

static output_t output_graph_fn = {
     .fingerprint_file = NULL,
     .options_set = (output_options_set_fn)output_graph_options_set,
     .default_options = (output_default_options_fn)output_graph_default_options,
     .set_options = (output_set_options_fn)output_graph_set_options,
     .prepare = (output_prepare_fn)output_graph_prepare,
     .display = (output_display_fn)output_graph_display,
};

static output_t *new_output_graph(logger_t log, input_t *input, const char *unit, int duration, time_fn time, increment_time_fn increment_time, value_fn value) {
     output_graph_t *result = malloc(sizeof(output_graph_t));
     result->fn = output_graph_fn;
     result->log = log;
     result->input = input;
     result->unit = unit;
     result->duration = duration;
     result->time = time;
     result->increment_time = increment_time;
     result->value = value;
     result->dict = cad_new_hash(stdlib_memory, cad_hash_strings);
     result->minh = result->minz = result->max = 0;
     result->keys = malloc(duration * sizeof(char*));
     memset(result->keys, 0, duration * sizeof(char*));
     memset(&(result->options), 0, sizeof(options_t));
     return &(result->fn);
}

static void normalize_time(struct tm*time) {
     time_t t;
     struct tm *ltime;
     time->tm_isdst = -1;
     t = mktime(time);
     ltime = localtime(&t);
     *time = *ltime;
}

static int year(output_graph_t *this, entry_t *entry) {
     int result = entry->year(entry);
     if (result == 0) {
          result = this->options.year;
     }
     return result - 1900;
}

static void second_time(output_graph_t *this, struct tm *time, entry_t *entry) {
     time->tm_sec  = entry->second(entry);
     time->tm_min  = entry->minute(entry);
     time->tm_hour = entry->hour(entry);
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry) - 1;
     time->tm_year = year(this, entry);
     time->tm_isdst = -1;
}

static void second_increment_time(struct tm*time) {
     time->tm_sec++;
     normalize_time(time);
}

static int second_value(struct tm*time) {
     return time->tm_sec;
}

static void minute_time(output_graph_t *this, struct tm *time, entry_t *entry) {
     time->tm_sec  = 0;
     time->tm_min  = entry->minute(entry);
     time->tm_hour = entry->hour(entry);
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry) - 1;
     time->tm_year = year(this, entry);
     time->tm_isdst = -1;
}

static void minute_increment_time(struct tm*time) {
     time->tm_min++;
     normalize_time(time);
}

static int minute_value(struct tm*time) {
     return time->tm_min;
}

static void hour_time(output_graph_t *this, struct tm *time, entry_t *entry) {
     time->tm_sec  = 0;
     time->tm_min  = 0;
     time->tm_hour = entry->hour(entry);
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry) - 1;
     time->tm_year = year(this, entry);
     time->tm_isdst = -1;
}

static void hour_increment_time(struct tm*time) {
     time->tm_hour++;
     normalize_time(time);
}

static int hour_value(struct tm*time) {
     return time->tm_hour;
}

static void day_time(output_graph_t *this, struct tm *time, entry_t *entry) {
     time->tm_sec  = 0;
     time->tm_min  = 0;
     time->tm_hour = 0;
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry) - 1;
     time->tm_year = year(this, entry);
     time->tm_isdst = -1;
}

static void day_increment_time(struct tm*time) {
     time->tm_mday++;
     normalize_time(time);
}

static int day_value(struct tm*time) {
     return time->tm_mday;
}

static void month_time(output_graph_t *this, struct tm *time, entry_t *entry) {
     time->tm_sec  = 0;
     time->tm_min  = 0;
     time->tm_hour = 0;
     time->tm_mday = 1;
     time->tm_mon  = entry->month(entry) - 1;
     time->tm_year = year(this, entry);
     time->tm_isdst = -1;
}

static void month_increment_time(struct tm*time) {
     time->tm_mon++;
     normalize_time(time);
}

static int month_value(struct tm*time) {
     return time->tm_mon + 1;
}

static void year_time(output_graph_t *this, struct tm *time, entry_t *entry) {
     time->tm_sec  = 0;
     time->tm_min  = 0;
     time->tm_hour = 0;
     time->tm_mday = 1;
     time->tm_mon  = 0;
     time->tm_year = year(this, entry);
     time->tm_isdst = -1;
}

static void year_increment_time(struct tm*time) {
     time->tm_year++;
     normalize_time(time);
}

static int year_value(struct tm*time) {
     return time->tm_year;
}

output_t *new_output_sgraph(logger_t log, input_t *input) {
     return new_output_graph(log, input, "second", 60, second_time, second_increment_time, second_value);
}

output_t *new_output_mgraph(logger_t log, input_t *input) {
     return new_output_graph(log, input, "minute", 60, minute_time, minute_increment_time, minute_value);
}

output_t *new_output_hgraph(logger_t log, input_t *input) {
     return new_output_graph(log, input, "hour", 24, hour_time, hour_increment_time, hour_value);
}

output_t *new_output_dgraph(logger_t log, input_t *input) {
     return new_output_graph(log, input, "day", 31, day_time, day_increment_time, day_value);
}

output_t *new_output_mograph(logger_t log, input_t *input) {
     return new_output_graph(log, input, "month", 12, month_time, month_increment_time, month_value);
}

output_t *new_output_ygraph(logger_t log, input_t *input) {
     return new_output_graph(log, input, "year", 10, year_time, year_increment_time, year_value);
}
