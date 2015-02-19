[![Build Status](https://travis-ci.org/cadrian/exp.png?branch=master)](https://travis-ci.org/cadrian/exp)

# General description

`ExP` is "Extra Petit". As an intended pun, *exp* is to *log* what `less` is to `more`. It is also a lot of regular *exp*ressions.

Ideas come from *Petit* (http://crunchtools.com/software/petit/).

That tool is really cool, but it is too slow on my small Cubieboard,
because it is written in Python. I need raw power `:-)` so I decided
to hack a clone in C.

Thanks Scott McCarty (*Petit*'s author) for the ideas and for allowing me.

`ExP` is designed as a drop-in replacement of *Petit*. Its internal
design also follows *Petit*'s with the following changes:

* allow multiple files
* per-log type filter scrubbing is externalized in configuration files
  instead of being hard-coded

# Work in progress

I am currently in the process of rewriting *Petit*. Almost everything
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
  * hash
  * wordcount
  * daemon
  * host

## Todo

* Entries
  * scriptlog
* Outputs
  * sgraph
  * mgraph
  * hgraph
  * dgraph
  * mograph
  * ygraph

# Future work

* Outsource entries regexps to be able to customize them; maybe allow
  more than one regexp per entry type
* Color graphs
* XDG support for easier development *and* standard deployment of
  configuration files
* Cumulated data in graphs: what happens *each* second, minute, etc.
