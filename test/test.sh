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

update=false
all=false
declare -a errors=()
case "$1" in
    update)
        update=true
        ;;
    all)
        all=true
        ;;
esac

# This function runs one test
function run_test() {
    local tst=$1
    local fun=$2
    local opt=$3

    local funarg=--$fun
    local optarg=${opt:+--$opt}
    local filename=$tst-$fun${opt:+-$opt}


    # Update files?
    if $update; then
        echo "Updating: petit $funarg $optarg $tst.log: "
        petit $funarg $optarg data/$tst.log >output/$filename.output 2>/dev/null
    fi

    # Run test
    echo -n "Testing: petit $funarg $optarg $tst.log: "
    petit $funarg $optarg data/$tst.log >$filename.tmp 2>$filename.log

    if diff -q output/$filename.output $filename.tmp >/dev/null; then
        rm $filename.{tmp,log}
        echo " Passed"
    else
        echo " FAILED"
        diff -u output/$filename.output $filename.tmp > $filename.diff
        # Leave data in place to inspect on failure #
        if $all; then
            errors=("${errors[@]}" "petit $funarg $optarg $tst.log")
        else
            cat $filename.diff
            exit 1
        fi
    fi
}

# First test with no input, should print version
if petit; then
    echo " Passed: Default with no input"
else
    echo " Failed: Default with no input"
    exit 1
fi

# Now run each function, some with extra options (see the list in the here-document below)
while read fun options; do
    for test in data/*.log; do
        run_test $(basename $test .log) $fun
        for opt in $options; do
            run_test $(basename $test .log) $fun $opt
        done
    done
done <<EOF
hash fingerprint nosample nofilter
wordcount
host
daemon
sgraph
mgraph
hgraph
dgraph
mograph
ygraph
EOF

errcount=${#errors[@]}
if [ $errcount -gt 0 ]; then
    echo "$errcount failed tests:"
    for error in "${errors[@]}"
    do
        echo "  $error"
    done
    exit 1
fi
