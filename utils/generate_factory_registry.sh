#!/usr/bin/env bash
#
# This file is part of ExP.
#
# ExP is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# ExP is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ExP.  If not, see <http://www.gnu.org/licenses/>.
#

cd $(dirname $(dirname $(readlink -f $0)))/src

{
    echo "/* File generated on $(date -R) */"
    echo '#include "exp_log.h"'
    echo '#include "exp_file.h"'
    echo '#include "exp_entry_factory.h"'
    echo

    factories=($(
        for entryfile in exp_*_entry.c; do
            egrep -o '^entry_factory_t *\* *new_([0-9A-Za-z_]+)_entry_factory\(logger_t +[[:alnum:]]+\) *{' $entryfile | egrep -o 'new_([0-9A-Za-z_]+)_entry_factory'
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
