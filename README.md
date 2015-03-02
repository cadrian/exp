[![Build Status](https://travis-ci.org/cadrian/exp.png?branch=master)](https://travis-ci.org/cadrian/exp)

# General description

## Introduction

`ExP` is "Extra Petit". As an intended pun, *exp* is to *log* what `less` is to `more`. It is also a lot of regular *exp*ressions.

Ideas come from [*Petit*](http://crunchtools.com/software/petit/).

That tool is really cool, but it is too slow on my small Cubieboard,
because it is written in Python. I need raw power `:-)` so I decided
to hack a clone in C.

Thanks Scott McCarty (*Petit*'s author) for the ideas and for allowing me.

## Dependencies

* [PCRE](http://www.pcre.org)
* [libCad](https://github.com/cadrian/libcad)

# Usage

## Compatibility

* Entries
  * syslog
  * rsyslog
  * apache access
  * apache error
  * securelog
  * snort
  * raw
* Outputs
  * hash
  * wordcount
  * daemon
  * host
  * sgraph
  * mgraph
  * hgraph
  * dgraph
  * mograph
  * ygraph

## Extensions

`ExP` is designed as a drop-in replacement of *Petit*. Its internal
design is also similar to *Petit*'s.

Yet, `ExP` has extentions of its own. Here are those extensions:

* Allow multiple files
* Per-log type filter scrubbing is externalized in configuration files
  instead of being hard-coded
* Graphs support a `--exp_mode` flag that extends the graph with a
  numeric scale for better legibility.
* The `--year` flag gives the "current year" for logs that don't
  support years. By default, `ExP` behaves like *Petit* and uses the
  current year.
* The `--filterdir` and `--fingerprintdir` allow to add extra
  directories to scan for resp. filters and fingerprints.

# Future work

* Outsource entries regexps to be able to customize them; maybe allow
  more than one regexp per entry type
* Color graphs
* XDG support for easier development *and* standard deployment of
  configuration files
* Cumulated data in graphs: what happens *each* second, minute, etc.
* Port `scriptlog` from *Petit*?
* Ideas from *Petit*
  * `--dev1` and `--dev2` sound cool
  * Missing fingerprints (Debian, etc.)
