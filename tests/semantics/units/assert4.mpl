var true : bool := (1=1);
var false : bool := (1=0);

assert(false < ((true&true) & (!false)));