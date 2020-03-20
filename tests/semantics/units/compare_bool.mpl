var true : bool := 8 < 10;
var false : bool := !true;

assert( (true = true) & (false = false) );
assert( false < true );