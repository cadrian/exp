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

#ifndef __EXP_ENTRY_SYSLOG_C__
#define __EXP_ENTRY_SYSLOG_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "exp.h"
#include "exp_regexp.h"

#include "_exp_entry_log.c"

static regexp_t *syslog_regexp(void) {
     static regexp_t *result = NULL;
     if (result == NULL) {
          result = new_regexp("^(?<date>(?<month>[A-Z][a-z]{2}) +(?<day>[0-9][0-9]?) +(?<hour>[0-9]{2}):(?<minute>[0-9]{2}):(?<second>[0-9]{2})) +(?<host>[^ ]+) +(?<daemon>[^ ]+): +(?<log>.*)$", 0, 10);
          if (result == NULL) {
               exit(1);
          }
     }
     return result;
}

static bool_t syslog_is_type(entry_factory_t *this, line_t *line) {
     bool_t result = false;
     regexp_t *regexp = syslog_regexp();
     match_t *match = regexp->match(regexp, line->buffer, 0, line->length, 0);
     const char *string;
     if (match != NULL) {
          string = match->named_substring(match, "daemon");
          if (string == NULL || (strlen(string) >= 5 && !strncmp("sshd[", string, 5))) {
               // don't match
          } else {
               string = match->named_substring(match, "log");
               if (string == NULL || (strlen(string) >= 4 && !strncmp("pam_", string, 4))) {
                    // don't match
               } else {
                    result = true;
               }
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
     const char *log;
} syslog_entry_t;

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

static const char *syslog_entry_log(syslog_entry_t *this) {
     return this->log;
}

static entry_t syslog_entry_fn = {
     .year   = (entry_year_fn  )syslog_entry_year  ,
     .month  = (entry_month_fn )syslog_entry_month ,
     .day    = (entry_day_fn   )syslog_entry_day   ,
     .hour   = (entry_hour_fn  )syslog_entry_hour  ,
     .minute = (entry_minute_fn)syslog_entry_minute,
     .second = (entry_second_fn)syslog_entry_second,
     .host   = (entry_host_fn  )syslog_entry_host  ,
     .daemon = (entry_daemon_fn)syslog_entry_daemon,
     .log    = (entry_log_fn   )syslog_entry_log   ,
};

static entry_t *syslog_new_entry(entry_factory_t *this, line_t *line) {
     syslog_entry_t *result = malloc(sizeof(syslog_entry_t));

     regexp_t *regexp = syslog_regexp();
     match_t *match = regexp->match(regexp, line->buffer, 0, line->length, 0);
     time_t tm;

     if (match != NULL) {
          tm = time(NULL);

          result->fn = syslog_entry_fn;

          result->year   = localtime(&tm)->tm_year + 1900;
          result->day    = atoi(match->named_substring(match, "day"));
          result->month  = month_of(match->named_substring(match, "month"));
          result->hour   = atoi(match->named_substring(match, "hour"));
          result->minute = atoi(match->named_substring(match, "minute"));
          result->second = atoi(match->named_substring(match, "second"));
          result->host   = strdup(match->named_substring(match, "host"));
          result->daemon = strdup(match->named_substring(match, "daemon"));
          result->log    = strdup(match->named_substring(match, "log"));

          match->free(match);
     } else {
          result->year = 1900;
          result->day = result->month = result->hour = result->minute = result->second = 1;
          result->host = result->daemon = "#";
          if (line->length > 0) {
               result->log = line->buffer;
          } else {
               result->log = "#";
          }
     }

     return &(result->fn);
}

static entry_factory_t syslog_entry_factory = {
     .tally_logic = log_tally_logic,
     .is_type = syslog_is_type,
     .new_entry = syslog_new_entry
};

#endif /* __EXP_ENTRY_SYSLOG_C__ */
