#!/usr/bin/env bash
#
# Written By: Scott McCarty <smccarty@eyemg.com>
# Date: 9/2009
# Description: Simple integration test to make sure things work rudamentarily well
#
# Modified By: Cyril Adrian <cyril.adrian@gmail.com>
# Date: 2/2015
# Description: Minimum changes to make it work with ExP; behaviour should be identical

cd $(dirname $(readlink -f $0))

(cd ..; exec make exe) || exit 1

fingerprintdir1=$(cd ../data/fingerprints; pwd)/
fingerprintdir2=$(cd ../data/fingerprint_library; pwd)/
filterdir=$(cd ../data/filters; pwd)/
petit() {
    ../target/exp -vv --fingerprintdir $fingerprintdir1 --fingerprintdir $fingerprintdir2 --filterdir $filterdir --year 2011 "$@"
}

################################################################################
# (no changes below)
#

# First test with no input, should print version
if ! petit
then
    echo " Failed: Default with no input"
    exit 1
else
    echo " Passed: Default with no input"
fi

# Routine Tests
functions="hash wordcount host daemon sgraph mgraph hgraph dgraph mograph ygraph"

declare -a errors=()

for function in $functions
do

    for test in `ls data/*.log`
    do
        # Get the right name for the test
        test=`basename $test | cut -f1 -d"."`

        # Update files?
        if [ "$1" == "update" ]
        then
            echo "Updating: petit --$function $test.log: "
            petit --${function} data/${test}.log > output/${test}-${function}.output 2>/dev/null
        fi

        # Run test
        echo -n "Testing: petit --$function $test.log: "
        petit --${function} data/${test}.log > ${test}-${function}.tmp 2> ${test}-${function}.log

        if ! diff output/${test}-${function}.output ${test}-${function}.tmp
        then
            echo " Failed"
            # Leave data in place to inspect on failure #
            if [ "$1" == "all" ]
            then
                errors=("${errors[@]}" "petit --$function $test.log")
            else
                exit 1
            fi
        else
            rm ${test}-${function}.{tmp,log}
            echo " Passed"
        fi

    done
done

# Special hashing tests

options="fingerprint nosample nofilter"
function="hash"

for option in $options
do
    for test in `ls data/*.log`
    do
        # Get the right name for the test
        test=`basename $test | cut -f1 -d"."`

        # Run test
        # Update files?
        if [ "$1" == "update" ]
        then
            echo "Updating: petit --$function --${option} $test.log: "
            petit --${function} --${option} data/${test}.log > output/${test}-${function}-${option}.output 2>/dev/null
        fi

        # Run Test
        echo -n "Testing: petit --$function --${option} $test.log: "
        petit --${function} --${option} data/${test}.log > ${test}-${function}-${option}.tmp 2> ${test}-${function}-${option}.log

        if ! diff output/${test}-${function}-${option}.output ${test}-${function}-${option}.tmp
        then
            echo " Failed"
            if [ "$1" == "all" ]
            then
                errors=("${errors[@]}" "petit --$function --${option} $test.log")
            else
                # Leave data in place to inspect on failure #
                exit 1
            fi
        else
            rm ${test}-${function}-${option}.{tmp,log}
            echo " Passed"
        fi
    done
done

errcount=${#errors[@]}
if [ $errcount -gt 0 ]; then
    echo "$errcount errors:"
    for error in "${errors[@]}"
    do
        echo "  $error"
    done
    exit 1
fi
