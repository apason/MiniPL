#!/bin/bash

#there is test.cfg file which contains one row per test. each row consists of
#two parts: name of the source file, and expected return value to environment.

#this test script runs the pregram with input of each file determined in test.cfg
#it cuts expected result and compares it with actual return value.

cd "$(dirname "$0")"

bin="../target/parser_test"

red='\033[0;31m'
green='\033[0;32m'
NC='\033[0m'

echo " "
echo "TESTING PARSER:"

for test in $(cat test.cfg | cut -f1 -d' '); do
    
    expected=$(cat test.cfg | grep $test | rev | cut -f1 -d' ' | rev);
    $bin units/$test >> /dev/null 2>&1 ;
    actual=$?;

    if [ "$actual" != "$expected" ] ; then
	echo -e test $test ${red} FAILED! ${NC} Expected $expected but was $actual;
    else
	echo -e test $test ${green} PASSED! ${NC};
    fi;

done
