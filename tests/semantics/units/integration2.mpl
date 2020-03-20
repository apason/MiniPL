/*
 * This program is used to test the lexical
 * analyzer of the miniPL interpreter.
 */
var nTimes : int := 0; //Test for inline comment
print "How many times?";
read nTimes;
var x : int;
/* test for multiline comment */
for x in 0..nTimes-1 do
    print x;
    print " : Hello, World!\n";
end for;
/* more */
// comments
assert (x = nTimes);
