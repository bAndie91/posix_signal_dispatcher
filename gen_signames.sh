#!/bin/sh

set -e
#set -o pipefail

echo "#include <signal.h>" |\
g++ -E -dD -xc++ - |\
perl -ne 'if(/^#define SIG([A-Z]+) (\d+|SIG[A-Z]+)/) { print "{ \"$1\", $2 },\n"; }'
