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
 * @ingroup exp_input
 * @file
 *
 * This file contains the implementation of all syslog-like entries and their factories.
 */

#ifndef __EXP_ENTRY_SYSLOG_C__
#define __EXP_ENTRY_SYSLOG_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cad_array.h>

#include "exp.h"
#include "exp_regexp.h"
#include "exp_entry.h"
#include "exp_entry_factory.h"

#define THIS_YEAR 0

static const char *dirs[] = {
     "/var/lib/exp/factories/",
     "/usr/local/exp/var/lib/factories/",
     "/opt/exp/var/lib/factories/",
     "/var/lib/petit/factories/",
     "/usr/local/petit/var/lib/factories/",
     "/opt/petit/var/lib/factories/",
     NULL
};

typedef struct syslog_entry_factory_s syslog_entry_factory_t;

typedef bool_t (*extra_is_type_fn)(syslog_entry_factory_t *this, match_t *match);

struct syslog_entry_factory_s {
     entry_factory_t fn;
     logger_t log;
     cad_array_t *regexps;
     regexp_t *regexp;
     const char *name;
     extra_is_type_fn extra_is_type;
     int priority;
     const char *default_host;
     const char *default_daemon;
     const char *default_logline;
     int default_year;
     const char * const*extradirs;
};

static regexp_t *raw_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp(log, "^(?<log>.*?)[[:space:]]*$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static regexp_t *space_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp(log, "[[:space:]]+", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static const char *syslog_factory_get_name(syslog_entry_factory_t *this) {
     return this->name;
}

static int syslog_factory_priority(syslog_entry_factory_t *this) {
     return this->priority;
}

static bool_t syslog_tally_logic(syslog_entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines) {
     bool_t result = tally > tally_threshold;
     this->log(debug, "%s tally_logic: %lu > %lu = %s\n", this->name, (unsigned long)tally, (unsigned long)tally_threshold, result?"true":"false");
     return result;
}

static bool_t securelog_tally_logic(syslog_entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines) {
     bool_t result = tally >= max_sample_lines;
     this->log(debug, "%s tally_logic: %lu > %lu = %s\n", this->name, (unsigned long)tally, (unsigned long)max_sample_lines, result?"true":"false");
     return result;
}

static bool_t securelog_extra_is_type(syslog_entry_factory_t *this, match_t *match) {
     bool_t result = false;
     const char *string;
     string = match->named_substring(match, "daemon");
     if (strlen(string) >= 5 && !strncmp("sshd[", string, 5)) {
          result = true;
     } else {
          string = match->named_substring(match, "log");
          if (strlen(string) >= 4 && !strncmp("pam_", string, 4)) {
               result = true;
          }
     }
     return result;
}

static bool_t syslog_extra_is_type(syslog_entry_factory_t *this, match_t *match) {
     return !securelog_extra_is_type(this, match);
}

static bool_t default_extra_is_type(syslog_entry_factory_t *this, match_t *match) {
     return true;
}

static bool_t read_regexps_(syslog_entry_factory_t *this, const char *dir, const char *filename, cad_array_t *regexps) {
     bool_t result = false;
     char *path = malloc(strlen(dir) + strlen(filename) + 1);
     file_t *file;
     line_t *line;
     regexp_t *regexp;
     int i, n;

     sprintf(path, "%s%s", dir, filename);
     file = new_file(this->log, debug, path);
     if (file != NULL) {
          result = true;
          n = file->lines_count(file);
          for (i = 0; i < n; i++) {
               line = file->line(file, i);
               regexp = new_regexp(this->log, line->buffer, 0);
               regexps->insert(regexps, regexps->count(regexps), &regexp);
          }
     }

     return result;
}

static cad_array_t *read_regexps(syslog_entry_factory_t *this) {
     cad_array_t *result = cad_new_array(stdlib_memory, sizeof(regexp_t *));
     bool_t found = false, f;
     static char filename[MAX_LINE_SIZE];
     const char *dir;
     int i;

     snprintf(filename, MAX_LINE_SIZE, "%s.factory", this->name);

     if (this->extradirs != NULL) {
          for (i = 0; (dir = this->extradirs[i]) != NULL; i++) {
               f = read_regexps_(this, dir, filename, result);
               found |= f;
          }
     }
     for (i = 0; (dir = dirs[i]) != NULL; i++) {
          f = read_regexps_(this, dir, filename, result);
          found |= f;
     }

     if (!found) {
          this->log(debug, "%s not found\n", filename);
     }
     return result;
}

static bool_t syslog_is_type(syslog_entry_factory_t *this, line_t *line) {
     bool_t result = false;
     cad_array_t *regexps = this->regexps;
     regexp_t *regexp;
     match_t *match;
     int i, n;

     if (regexps == NULL) {
          regexps = this->regexps = read_regexps(this);
     }
     this->regexp = NULL;

     n = regexps->count(regexps);
     for (i = 0; !result && i < n; i++) {
          regexp = *(regexp_t **)regexps->get(regexps, i);
          match = regexp->match(regexp, line->buffer, 0, line->length, 0);
          if (match != NULL) {
               result = this->extra_is_type(this, match);
               if (result) {
                    this->regexp = regexp;
               }
               match->free(match);
          }
     }

     return result;
}

typedef struct {
     entry_t fn;
     int year  ;
     int month ;
     int day   ;
     int hour  ;
     int minute;
     int second;
     const char *host;
     const char *daemon;
     const char *logline;
     logger_t log;
     const char *name;
} syslog_entry_t;

static const char *syslog_get_name(syslog_entry_t *this) {
     return this->name;
}

static int syslog_entry_year(syslog_entry_t *this) {
     return this->year;
}

static int syslog_entry_month(syslog_entry_t *this) {
     return this->month;
}

static int syslog_entry_day(syslog_entry_t *this) {
     return this->day;
}

static int syslog_entry_hour(syslog_entry_t *this) {
     return this->hour;
}

static int syslog_entry_minute(syslog_entry_t *this) {
     return this->minute;
}

static int syslog_entry_second(syslog_entry_t *this) {
     return this->second;
}

static const char *syslog_entry_host(syslog_entry_t *this) {
     return this->host;
}

static const char *syslog_entry_daemon(syslog_entry_t *this) {
     return this->daemon;
}

static const char *syslog_entry_logline(syslog_entry_t *this) {
     return this->logline;
}

static entry_t syslog_entry_fn = {
     .get_name = (entry_get_name_fn)syslog_get_name     ,
     .year     = (entry_year_fn    )syslog_entry_year   ,
     .month    = (entry_month_fn   )syslog_entry_month  ,
     .day      = (entry_day_fn     )syslog_entry_day    ,
     .hour     = (entry_hour_fn    )syslog_entry_hour   ,
     .minute   = (entry_minute_fn  )syslog_entry_minute ,
     .second   = (entry_second_fn  )syslog_entry_second ,
     .host     = (entry_host_fn    )syslog_entry_host   ,
     .daemon   = (entry_daemon_fn  )syslog_entry_daemon ,
     .logline  = (entry_logline_fn )syslog_entry_logline,
};

static int string_2_int(syslog_entry_factory_t *this, match_t *match, const char *field, int (*deflt)(syslog_entry_factory_t*,match_t*)) {
     int result;
     const char *value = match->named_substring(match, field);
     if (value == NULL) {
          result = deflt(this, match);
     } else {
          result = atoi(value);
     }
     return result;
}

static int one(syslog_entry_factory_t *this, match_t *match) {
     return 1;
}

static int default_year(syslog_entry_factory_t *this, match_t *match) {
     return this->default_year;
}

static int str_month(syslog_entry_factory_t *this, match_t *match) {
     int result;
     const char *strmonth = match->named_substring(match, "strmonth");
     if (strmonth == NULL) {
          result = 1;
     } else {
          result = month_of(strmonth);
     }
     return result;
}

static char *string_clone(const char *string, const char *def) {
     char *result;
     if (string == NULL) {
          result = def == NULL ? NULL : strdup(def);
     } else {
          result = strdup(string);
     }
     return result;
}

static entry_t *syslog_new_entry(syslog_entry_factory_t *this, line_t *line) {
     syslog_entry_t *result = malloc(sizeof(syslog_entry_t));

     regexp_t *re_sp = space_regexp(this->log);
     regexp_t *re_raw = raw_regexp(this->log);
     match_t *match = NULL;
     char *logline = NULL;

     result->fn = syslog_entry_fn;
     result->name    = this->name;
     result->log     = this->log;

     if (this->regexp == NULL && !syslog_is_type(this, line)) {
          this->log(warn, "Incompatible factory %s | %.*s\n", this->name, (int)line->length, line->buffer);
     } else {
          match = this->regexp->match(this->regexp, line->buffer, 0, line->length, 0);
     }

     if (match != NULL) {
          result->year    = string_2_int(this, match, "year",   default_year);
          result->month   = string_2_int(this, match, "month",  str_month);
          result->day     = string_2_int(this, match, "day",    one);
          result->hour    = string_2_int(this, match, "hour",   one);
          result->minute  = string_2_int(this, match, "minute", one);
          result->second  = string_2_int(this, match, "second", one);
          result->host    = string_clone(match->named_substring(match, "host"), this->default_host);
          result->daemon  = string_clone(match->named_substring(match, "daemon"), this->default_daemon);
          logline         = string_clone(match->named_substring(match, "log"), NULL);
          match->free(match);
     } else {
          result->year = 1900;
          result->day = result->month = result->hour = result->minute = result->second = 1;
          result->host = result->daemon = "#";
          if (line->length > 0) {
               match = re_raw->match(re_raw, line->buffer, 0, line->length, 0);
               if (match != NULL) {
                    logline = string_clone(match->named_substring(match, "log"), NULL);
                    match->free(match);
               } else {
                    logline = string_clone(line->buffer, NULL);
               }
          }
     }
     if (logline == NULL) {
          result->logline = this->default_logline;
     } else {
          re_sp->replace_all(re_sp, " ", logline);
          result->logline = logline;
     }
     return &(result->fn);
}

static void syslog_set_extradirs(syslog_entry_factory_t *this, const char * const*extradirs) {
     this->extradirs = extradirs;
}

static entry_factory_t syslog_entry_factory_fn = {
     .get_name = (entry_factory_get_name_fn)syslog_factory_get_name,
     .priority = (entry_factory_priority_fn)syslog_factory_priority,
     .tally_logic = (entry_factory_tally_logic_fn)syslog_tally_logic,
     .is_type = (entry_factory_is_type_fn)syslog_is_type,
     .new_entry = (entry_factory_new_entry_fn)syslog_new_entry,
     .set_extradirs = (entry_factory_set_extradirs_fn)syslog_set_extradirs,
};

entry_factory_t *new_syslog_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexps = NULL;
     result->regexp = NULL;
     result->name = "syslog";
     result->extra_is_type = syslog_extra_is_type;
     result->priority = 1;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = THIS_YEAR;
     return &(result->fn);
}

entry_factory_t *new_rsyslog_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexps = NULL;
     result->regexp = NULL;
     result->name = "rsyslog";
     result->extra_is_type = default_extra_is_type;
     result->priority = 1;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = THIS_YEAR;
     return &(result->fn);
}

entry_factory_t *new_apache_access_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexps = NULL;
     result->regexp = NULL;
     result->name = "apache_access";
     result->extra_is_type = default_extra_is_type;
     result->priority = 0;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = THIS_YEAR;
     return &(result->fn);
}

entry_factory_t *new_apache_error_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexps = NULL;
     result->regexp = NULL;
     result->name = "apache_error";
     result->extra_is_type = default_extra_is_type;
     result->priority = 0;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = THIS_YEAR;
     return &(result->fn);
}

entry_factory_t *new_securelog_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->fn.tally_logic = (entry_factory_tally_logic_fn)securelog_tally_logic;
     result->log = log;
     result->regexps = NULL;
     result->regexp = NULL;
     result->name = "securelog";
     result->extra_is_type = securelog_extra_is_type;
     result->priority = 2;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = THIS_YEAR;
     return &(result->fn);
}

entry_factory_t *new_snort_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexps = NULL;
     result->regexp = NULL;
     result->name = "snort";
     result->extra_is_type = default_extra_is_type;
     result->priority = 0;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = THIS_YEAR;
     return &(result->fn);
}

entry_factory_t *new_raw_entry_factory(logger_t log) { /* is it used? */
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     regexp_t *regexp = raw_regexp(log);
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexps = cad_new_array(stdlib_memory, sizeof(regexp_t *));
     result->regexps->insert(result->regexps, 0, &regexp);
     result->regexp = NULL;
     result->name = "raw";
     result->extra_is_type = default_extra_is_type;
     result->priority = -1;
     result->default_host = "";
     result->default_daemon = "";
     result->default_logline = "#";
     result->default_year = 1900;
     return &(result->fn);
}

#endif /* __EXP_ENTRY_SYSLOG_C__ */
