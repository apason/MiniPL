var true : bool := (1=1);
var false : bool := !(1=1);

assert(true = (true&true));
assert(false = (false&false));
assert(false = (true&false));