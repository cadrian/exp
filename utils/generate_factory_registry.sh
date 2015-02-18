#!/usr/bin/env bash

cd $(dirname $(dirname $(readlink -f $0)))/src

{
    echo "/* File generated on $(date -R) */"
    echo '#include "exp_log.h"'
    echo '#include "exp_entry_factory.h"'
    echo

    for entryfile in exp_*_entry.c; do
        entry=$(echo $entryfile | sed -r 's/exp_([^_]+)_entry\.c/\1/')
        echo "extern entry_factory_t *new_${entry}_entry_factory(logger_t);"
    done
    echo

    echo "void register_all_factories(logger_t log) {"
    echo "    static bool_t registered = false;"
    echo "    if (!registered) {"
    echo "        registered = true;"
    for entryfile in exp_*_entry.c; do
        entry=$(echo $entryfile | sed -r 's/exp_([^_]+)_entry\.c/\1/')
        echo "        register_factory(new_${entry}_entry_factory(log));"
    done
    echo "    }"
    echo "}"

} > _exp_entry_registry.c
