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

#include "exp.h"
#include "exp_regexp.h"
#include "exp_entry.h"
#include "exp_entry_factory.h"

typedef struct syslog_entry_factory_s syslog_entry_factory_t;

typedef regexp_t *(*regexp_fn)(logger_t log);
typedef bool_t (*extra_is_type_fn)(syslog_entry_factory_t *this, match_t *match);

struct syslog_entry_factory_s {
     entry_factory_t fn;
     logger_t log;
     regexp_fn regexp;
     const char *name;
     extra_is_type_fn extra_is_type;
};

static regexp_t *syslog_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp(log,
                              "^(?<date>(?<strmonth>[A-Z][a-z]{2}) +(?<day>[0-9][0-9]?) +(?<hour>[0-9]{2}):(?<minute>[0-9]{2}):(?<second>[0-9]{2})) +"
                              "(?<host>[^ ]+) +(?<daemon>[^ ]+): +(?<log>.*)$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static regexp_t *rsyslog_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp(log,
                              "^(?<date>(?<year>[0-9]{4})-(?<month>[0-9]{2})-(?<day>[0-9]{2})T(?<hour>[0-9]{2}):(?<minute>[0-9]{2}):(?<second>[0-9]{2})\\.[0-9]+[-+][0-9]{2}:[0-9]{2}) +"
                              "(?<host>[^ ]+) +(?<daemon>[^ ]+): +(?<log>.*)$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static regexp_t *apache_access_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          // 127.0.0.1 - frank [10/Oct/2000:13:55:36 -0700] "GET /apache_pb.gif HTTP/1.0" 200 2326
          result = new_regexp(log,
                              "^(?<ip>[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}) - (?<user>[^ ]*) - "
                              "\\[(?<date>(?<day>[0-9][0-9]?)/+(?<strmonth>[A-Z][a-z]{2})/(?<year>[0-9]{4}):(?<hour>[0-9]{2}):(?<minute>[0-9]{2}):(?<second>[0-9]{2}) +[-+][0-9]{4})\\] +"
                              "(?<log>\"(?<query>[^\"]+)\" +(?<status>[0-9]{3}) +(?<contentlength>([0-9]+|-)))$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static regexp_t *apache_error_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          // [Wed Oct 11 14:32:52 2000] [error] [client 127.0.0.1] client denied by server configuration: /export/home/live/ap/htdocs/test
          result = new_regexp(log,
                              "^\\[(?<date>(?<strday>[A-Z][a-z]{2}) +(?<strmonth>[A-Z][a-z]{2}) +(?<day>[0-9][0-9]?) +(?<hour>[0-9]{2}):(?<minute>[0-9]{2}):(?<second>[0-9]{2}) +(?<year>[0-9]{4}))\\] +"
                              "(?<log>\\[(?<level>[a-z]+)\\] +\\[(?<client>[^]]+)\\] +(?<message>.*))$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static regexp_t *snort_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp(log,
                              "^(?<date>(?<month>[0-9]{2})/(?<day>[0-9]{2}):(?<hour>[0-9]{2}):(?<minute>[0-9]{2}):(?<second>[0-9]{2})\\.[0-9]+) +"
                              "(?<log>.*)$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static regexp_t *raw_regexp(logger_t log) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp(log,
                              "^(?<log>.*)$", 0);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static const char *syslog_factory_get_name(syslog_entry_factory_t *this) {
     return this->name;
}

static bool_t syslog_tally_logic(syslog_entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines) {
     return tally > tally_threshold;
}

static bool_t securelog_tally_logic(syslog_entry_factory_t *this, size_t tally, size_t tally_threshold, size_t max_sample_lines) {
     return tally > max_sample_lines;
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

static bool_t syslog_is_type(syslog_entry_factory_t *this, line_t *line) {
     bool_t result = false;
     regexp_t *regexp = this->regexp(this->log);
     match_t *match = regexp->match(regexp, line->buffer, 0, line->length, 0);
     if (match != NULL) {
          if (this->extra_is_type == NULL) {
               result = true;
          } else {
               result = this->extra_is_type(this, match);
          }
          match->free(match);
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

static int string_2_int(match_t *match, const char *field, int (*deflt)(match_t*)) {
     int result;
     const char *value = match->named_substring(match, field);
     if (value == NULL) {
          result = deflt(match);
     } else {
          result = atoi(value);
     }
     return result;
}

static int this_year(match_t *match) {
     int result;
     time_t tm;
     static bool_t init = false;
     if (!init) {
          init = true;
          tm = time(NULL);
          result = localtime(&tm)->tm_year + 1900;
     }
     return result;
}

static int one(match_t *match) {
     return 1;
}

static int str_month(match_t *match) {
     int result;
     const char *strmonth = match->named_substring(match, "strmonth");
     if (strmonth == NULL) {
          result = 1;
     } else {
          result = month_of(strmonth);
     }
     return result;
}

static char *string_clone(const char *string) {
     char *result;
     if (string == NULL) {
          result = NULL;
     } else {
          result = strdup(string);
     }
     return result;
}

static entry_t *syslog_new_entry(syslog_entry_factory_t *this, line_t *line) {
     syslog_entry_t *result = malloc(sizeof(syslog_entry_t));

     regexp_t *regexp = this->regexp(this->log);
     match_t *match = regexp->match(regexp, line->buffer, 0, line->length, 0);

     if (match != NULL) {
          result->fn = syslog_entry_fn;

          result->year    = string_2_int(match, "year",   this_year);
          result->month   = string_2_int(match, "month",  str_month);
          result->day     = string_2_int(match, "day",    one);
          result->hour    = string_2_int(match, "hour",   one);
          result->minute  = string_2_int(match, "minute", one);
          result->second  = string_2_int(match, "second", one);
          result->host    = string_clone(match->named_substring(match, "host"));
          result->daemon  = string_clone(match->named_substring(match, "daemon"));
          result->logline = string_clone(match->named_substring(match, "log"));

          result->log     = this->log;
          result->name    = this->name;

          match->free(match);
     } else {
          result->year = 1900;
          result->day = result->month = result->hour = result->minute = result->second = 1;
          result->host = result->daemon = "#";
          if (line->length > 0) {
               result->logline = line->buffer;
          } else {
               result->logline = "#";
          }
     }

     return &(result->fn);
}

static entry_factory_t syslog_entry_factory_fn = {
     .get_name = (entry_factory_get_name_fn)syslog_factory_get_name,
     .tally_logic = (entry_factory_tally_logic_fn)syslog_tally_logic,
     .is_type = (entry_factory_is_type_fn)syslog_is_type,
     .new_entry = (entry_factory_new_entry_fn)syslog_new_entry
};

entry_factory_t *new_syslog_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexp = syslog_regexp;
     result->name = "syslog";
     result->extra_is_type = syslog_extra_is_type;
     return &(result->fn);
}

entry_factory_t *new_rsyslog_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexp = rsyslog_regexp;
     result->name = "rsyslog";
     result->extra_is_type = NULL;
     return &(result->fn);
}

entry_factory_t *new_apache_access_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexp = apache_access_regexp;
     result->name = "apache_access";
     result->extra_is_type = NULL;
     return &(result->fn);
}

entry_factory_t *new_apache_error_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexp = apache_error_regexp;
     result->name = "apache_error";
     result->extra_is_type = NULL;
     return &(result->fn);
}

entry_factory_t *new_securelog_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->fn.tally_logic = (entry_factory_tally_logic_fn)securelog_tally_logic;
     result->log = log;
     result->regexp = syslog_regexp;
     result->name = "securelog";
     result->extra_is_type = securelog_extra_is_type;
     return &(result->fn);
}

entry_factory_t *new_snort_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexp = snort_regexp;
     result->name = "snort";
     result->extra_is_type = NULL;
     return &(result->fn);
}

entry_factory_t *new_raw_entry_factory(logger_t log) {
     syslog_entry_factory_t *result = malloc(sizeof(syslog_entry_factory_t));
     result->fn = syslog_entry_factory_fn;
     result->log = log;
     result->regexp = raw_regexp;
     result->name = "raw";
     result->extra_is_type = NULL;
     return &(result->fn);
}

#endif /* __EXP_ENTRY_SYSLOG_C__ */
