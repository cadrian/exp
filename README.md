[![Build Status](https://travis-ci.org/cadrian/exp.png?branch=master)](https://travis-ci.org/cadrian/exp)

# General description

`exp` is to *log* what `less` is to `more`.

Also, exp is "Extra Petit".

Ideas come from *petit* (http://crunchtools.com/software/petit/). That
tool is really cool, but it is way too slow on my small Cubieboard,
because it is written in Python. Also, petit's last commit is getting
old. Is it still maintained?

That's why I wanted to code a clone in C.

`exp` is designed as a drop-in replacement of *petit*. Its internal
design also follows *petit*'s with the following changes:

* allow multiple filenames
* per-log type filter scrubbing is externalized in configuration files
  instead of being hard-coded
