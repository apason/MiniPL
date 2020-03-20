// Test for string addition and empty strings
var s1 : string;
var s2 : string;
var s3 : string := "Hello ";
var s4 : string := "Hello World!";

var s5 : string := s3 + s1; // "Hello "

s1 := s5;                   // "Hello "
s2 := "World!";             // "Wordl"
s3 := "";                   // ""

s3 := s3 + s1;              // "Hello "
assert((("" + s3) + (("" + "World!")+ "")) = "Hello World!");
assert((s3 + s2) = s4);