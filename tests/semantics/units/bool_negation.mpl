var true : bool := (1=1);
var false : bool := !(1=1);

assert(true = (!false));
assert(false = (!true));
