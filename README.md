[![Build Status](https://travis-ci.org/cadrian/exp.png?branch=master)](https://travis-ci.org/cadrian/exp)

# General description: what is ExP?

`ExP` is "Extra Petit". As an intended pun, *exp* is to *log* what `less` is to `more`. It is also a lot of regular *exp*ressions.

Ideas come from *petit* (http://crunchtools.com/software/petit/). That
tool is really cool, but it is way too slow on my small Cubieboard,
because it is written in Python. Finally, *petit*'s last commit is
getting old. Is it still maintained?

That's why I wanted to code a clone in C.

`ExP` is designed as a drop-in replacement of *petit*. Its internal
design also follows *petit*'s with the following changes:

* allow multiple files
* per-log type filter scrubbing is externalized in configuration files
  instead of being hard-coded

# Work in progress

I am currently in the process of rewriting *petit*. Almost everything
will be ported.

## Done

* Entries
  * syslog
  * rsyslog
  * apache access
  * apache error
  * securelog
  * snort
  * raw
* Outputs
  * hash (**todo: fingerprint**)

## Todo

* Entries
  * scriptlog
* Outputs
  * wordcount
  * daemon
  * host
  * sgraph
  * mgraph
  * hgraph
  * dgraph
  * mograph
  * ygraph

# Future work

* Outsource entries regexps to be able to customize them; maybe allow
  more than one regexp per entry type
