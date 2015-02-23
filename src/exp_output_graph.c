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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <cad_hash.h>

#include "exp_output.h"
#include "exp_file.h"

#define DEFAULT_GRAPH_HEIGHT 6

typedef struct output_graph_s output_graph_t;
typedef void (*time_fn)(struct tm *time, entry_t *entry);
typedef void (*increment_time_fn)(struct tm *time);
typedef int (*value_fn)(struct tm *time);

struct output_graph_s {
     output_t fn;
     logger_t log;
     input_t *input;
     output_options_t options;
     const char *unit;
     int duration;
     time_fn time;
     increment_time_fn increment_time;
     value_fn value;
     cad_hash_t *dict;
     int min;
     int max;
     struct tm start, middle, end;
     char **keys;
};

static const char* strdate(struct tm*time) {
     static char result[128];
     strftime(result, 128, "%Y/%m/%d %H:%M:%S", time);
     return result;
}

static void graph_prepare_min_max(cad_hash_t *hash, int index, const char *key, int *value, output_graph_t *this) {
     if (*value > 0) {
          if (this->min > *value) {
               this->min = *value;
          }
          if (this->max < *value) {
               this->max = *value;
          }
     }
}

static int cmpstr(const char **a, const char **b) {
     return strcmp(*a, *b);
}

static void graph_prepare(output_graph_t *this) {
     struct tm current = {0,};
     bool_t is_first = true;
     input_file_t *file;
     entry_t *entry;
     int i, n = this->input->files_length(this->input), j, m;
     const char *key;
     int *value;

     this->start = this->middle = this->end = current;

     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          if (file->entries_length(file) > 0) {
               entry = file->entry(file, 0);
               this->time(&current, entry);
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
          value = malloc(sizeof(int));
          *value = 0;
          this->dict->set(this->dict, key, value);
          this->keys[i] = strdup(key);
     }
     this->end = current;
     qsort(this->keys, this->duration, sizeof(char*), (int(*)(const void*, const void*))cmpstr);

     for (i = 0; i < n; i++) {
          file = this->input->file(this->input, i);
          m = file->entries_length(file);
          for (j = 0; j < m;j++) {
               entry = file->entry(file, j);
               this->time(&current, entry);
               key = strdate(&current);
               value = this->dict->get(this->dict, key);
               if (value != NULL) {
                    *value = (*value) + 1;
               }
          }
     }
     this->dict->iterate(this->dict, (cad_hash_iterator_fn)graph_prepare_min_max, this);
     this->log(info, "Graph: dates from %s to %s, occurrences between %d and %d\n", strdate(&(this->start)), strdate(&(this->end)), this->min, this->max);
}

typedef struct {
     logger_t log;
     int height;
     int width;
     int min;
     int max;
} graph_t;

static void graph_normalize(cad_hash_t *hash, int index, const char *key, int *value, graph_t *graph) {
     int normalized;
     if (*value > 0) {
          if (graph->max > graph->min) {
               normalized = (int)(ceil((double)(*value - graph->min) * (double)graph->height / (double)(graph->max - graph->min)));
          } else {
               normalized = (int)(ceil((double)(*value) * (double)graph->height / (double)(graph->max)));
          }
          graph->log(debug, "normalized: %d -> %d\n", *value, normalized);
          *value = normalized;
     }
}

static void graph_display(output_graph_t *this) {
     graph_t graph = {
          .log = this->log,
          .height = DEFAULT_GRAPH_HEIGHT,
          .width = this->duration,
          .min = this->min,
          .max = this->max,
     };
     double scale;
     char char_fill[4];
     char char_blank[4];
     int y, i;

     int graph_position_start, graph_position_middle, graph_position_end;
     int graph_width;
     const char *key;
     int *value;

     if (this->min < this->max) {
          scale = (double)(this->max - this->min) / (double)graph.height;
     } else {
          scale = (double)this->max / (double)graph.height;
     }
     this->dict->iterate(this->dict, (cad_hash_iterator_fn)graph_normalize, &graph);

     if (this->options.wide) {
          sprintf(char_fill, "%c%c", this->options.tick[0], this->options.tick[1] == '\0' ? ' ' : this->options.tick[1]);
          strcpy(char_blank, "  ");
          graph_width = graph.width * 2;
          graph_position_start = 1;
          graph_position_middle = graph.width - (graph.width % 2);
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

     fputc('\n', stdout);
     for (y = 0; y < graph.height; y++) {
          for (i = 0; i < this->duration; i++) {
               key = this->keys[i];
               value = this->dict->get(this->dict, key);
               if (value != NULL) {
                    if (*value > y) {
                         fputs(char_fill, stdout);
                    } else {
                         fputs(char_blank, stdout);
                    }
               }
          }
          fputc('\n', stdout);
     }

     for (i = 0; i < this->duration; i++) {
          fputs(char_fill, stdout);
     }
     fputc('\n', stdout);

     for (i = 0; i < graph_width; i++) {
          if (i+1 == graph_position_start) {
               printf("%.2d", this->value(&(this->start)) % 2000);
          } else if (i+1 == graph_position_middle) {
               printf("%.2d", this->value(&(this->middle)) % 2000);
          } else if (i+1 == graph_position_end) {
               printf("%.2d", this->value(&(this->end)) % 2000);
          } else {
               fputc(' ', stdout);
          }
     }
     fputc('\n', stdout);

     fputc('\n', stdout);
     printf("Start time:\t%s\tMinimum value:%d\n", strdate(&(this->start)), this->min);
     printf("End time:\t%s\tMaximum value:%d\n", strdate(&(this->end)), this->max);
     printf("Duration:\t%d %ss\t\t\tScale:%.2g\n", this->duration, this->unit, scale);
     fputc('\n', stdout);
}

static void output_graph_display(output_graph_t *this) {
     graph_prepare(this);
     graph_display(this);
}

static options_set_t output_graph_options_set(output_graph_t *this) {
     static options_set_t result = {
          false, false, true, true, false,
     };
     return result;
}

static output_t output_graph_fn = {
     .fingerprint_file = NULL,
     .options_set = (output_options_set_fn)output_graph_options_set,
     .display = (output_display_fn)output_graph_display,
};

static output_t *new_output_graph(logger_t log, input_t *input, output_options_t options, const char *unit, int duration, time_fn time, increment_time_fn increment_time, value_fn value) {
     output_graph_t *result = malloc(sizeof(output_graph_t));
     result->fn = output_graph_fn;
     result->log = log;
     result->input = input;
     result->options = options;
     result->unit = unit;
     result->duration = duration;
     result->time = time;
     result->increment_time = increment_time;
     result->value = value;
     result->dict = cad_new_hash(stdlib_memory, cad_hash_strings);
     result->min = INT_MAX;
     result->max = 0;
     result->keys = malloc(duration * sizeof(char*));
     memset(result->keys, 0, duration * sizeof(char*));
     return &(result->fn);
}

static void normalize_time(struct tm*time) {
     time_t t = mktime(time);
     struct tm *ltime = localtime(&t);
     *time = *ltime;
}

static void second_time(struct tm *time, entry_t *entry) {
     time->tm_sec  = entry->second(entry);
     time->tm_min  = entry->minute(entry);
     time->tm_hour = entry->hour(entry);
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry);
     time->tm_year = entry->year(entry);
}

static void second_increment_time(struct tm*time) {
     time->tm_sec++;
     normalize_time(time);
}

static int second_value(struct tm*time) {
     return time->tm_sec;
}

static void minute_time(struct tm *time, entry_t *entry) {
     time->tm_min  = entry->minute(entry);
     time->tm_hour = entry->hour(entry);
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry);
     time->tm_year = entry->year(entry);
}

static void minute_increment_time(struct tm*time) {
     time->tm_min++;
     normalize_time(time);
}

static int minute_value(struct tm*time) {
     return time->tm_min;
}

static void hour_time(struct tm *time, entry_t *entry) {
     time->tm_hour = entry->hour(entry);
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry);
     time->tm_year = entry->year(entry);
}

static void hour_increment_time(struct tm*time) {
     time->tm_hour++;
     normalize_time(time);
}

static int hour_value(struct tm*time) {
     return time->tm_hour;
}

static void day_time(struct tm *time, entry_t *entry) {
     time->tm_mday = entry->day(entry);
     time->tm_mon  = entry->month(entry);
     time->tm_year = entry->year(entry);
}

static void day_increment_time(struct tm*time) {
     time->tm_mday++;
     normalize_time(time);
}

static int day_value(struct tm*time) {
     return time->tm_mday;
}

static void month_time(struct tm *time, entry_t *entry) {
     time->tm_mon  = entry->month(entry);
     time->tm_year = entry->year(entry);
}

static void month_increment_time(struct tm*time) {
     time->tm_mon++;
     normalize_time(time);
}

static int month_value(struct tm*time) {
     return time->tm_mon;
}

static void year_time(struct tm *time, entry_t *entry) {
     time->tm_year = entry->year(entry);
}

static void year_increment_time(struct tm*time) {
     time->tm_year++;
     normalize_time(time);
}

static int year_value(struct tm*time) {
     return time->tm_year;
}

output_t *new_output_sgraph(logger_t log, input_t *input, output_options_t options) {
     return new_output_graph(log, input, options, "second", 60, second_time, second_increment_time, second_value);
}

output_t *new_output_mgraph(logger_t log, input_t *input, output_options_t options) {
     return new_output_graph(log, input, options, "minute", 60, minute_time, minute_increment_time, minute_value);
}

output_t *new_output_hgraph(logger_t log, input_t *input, output_options_t options) {
     return new_output_graph(log, input, options, "hour", 24, hour_time, hour_increment_time, hour_value);
}

output_t *new_output_dgraph(logger_t log, input_t *input, output_options_t options) {
     return new_output_graph(log, input, options, "day", 31, day_time, day_increment_time, day_value);
}

output_t *new_output_mograph(logger_t log, input_t *input, output_options_t options) {
     return new_output_graph(log, input, options, "month", 12, month_time, month_increment_time, month_value);
}

output_t *new_output_ygraph(logger_t log, input_t *input, output_options_t options) {
     return new_output_graph(log, input, options, "year", 10, year_time, year_increment_time, year_value);
}
