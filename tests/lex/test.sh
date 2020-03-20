#!/bin/bash

#there is test.cfg file which contains one row per test. each row consists of
#two parts: name of the source file, and expected sequenqe of tokens found.

#token sequence uses same values as the main program uses.
#full list of token types are declared in src/lex.h

#this test script runs the pregram with input of each file determined in test.cfg
#it cuts types of output token stream and compares it with expected value.

cd "$(dirname "$0")"

bin="../target/lex_test"

red='\033[0;31m'
green='\033[0;32m'
NC='\033[0m'

echo " "
echo "TESTING LEXICAL ANALYZER:"

for test in $(cat test.cfg | cut -f1 -d' '); do
    
    expected=$(cat test.cfg | grep $test | rev | cut -f1 -d' ' | rev)20;
    actual=$($bin units/$test | cut -f2 | sed ':a;N;$!ba;s/\n//g');

    if [ "$actual" != "$expected" ] ; then
	echo -e test $test ${red} FAILED! ${NC} Expected $expected but was $actual;
    else
	echo -e test $test ${green} PASSED! ${NC};
    fi;

done

