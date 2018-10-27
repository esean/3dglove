#!/bin/bash

#
# USAGE: $0 [dir to be moved] [directory to move to]
#

die() { echo -e "\nERROR: $0: $@ !" >&2; exit 1; }
[[ -z $1 ]] || [[ -z $2 ]] && die "USAGE `grep ^# $0`"
debug=1

#
cpone() {
    if [ $debug -eq 1 ]; then
        echo "cpone($@)"
        return
    fi

    from="$1"
    to="$2"

    [ ! -d $from ] && die "No dir $from"
    [ ! -d $to ] && die "No dir $to"

    cp -a $from $to
    svn add $to/$from
    svn rm $from

    echo "svn commit $from $to/$from"
}
dbg() { [ $debug -eq 1 ] && echo "DBG:$@"; }

txt=""
NXT="$1"
T='/tmp/out2'
rm -f $T

while [ ! -z "$NXT" ]; do

    echo "INSPECT: $NXT($@)"
    if [ -z "$NXT" ]; then
        dbg "NXT - cpone $NXT $2 &> $T"
    else
        dbg "cpone $NXT $2 &> $T"
        cpone "$NXT" "$2" &> $T
    fi

    shift
    NXT="$1"

done

