#!/usr/bin/env bash

cd $(dirname $(dirname $(readlink -f $0)))/src

{
    echo "/* File generated on $(date -R) */"
    echo '#include "exp_log.h"'
    echo '#include "exp_file.h"'
    echo '#include "exp_entry_factory.h"'
    echo

    factories=($(
        for entryfile in exp_*.c; do
            egrep -o '^entry_factory_t *\* *new_[^_]+_entry_factory\(logger_t +[[:alnum:]]+\) *{' $entryfile | egrep -o 'new_[^_]+_entry_factory'
        done
    ))

    for factory in "${factories[@]}"; do
        echo "extern entry_factory_t *${factory}(logger_t);"
    done
    echo

    echo "void register_all_factories(logger_t log) {"
    echo "    static bool_t registered = false;"
    echo "    if (!registered) {"
    echo "        registered = true;"
    for factory in "${factories[@]}"; do
        echo "        register_factory($factory(log));"
    done
    echo "    }"
    echo "}"

} > _exp_entry_registry.c
