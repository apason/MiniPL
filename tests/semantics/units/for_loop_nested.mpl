var i : int ;
var j : int ;
var sum : int := 0;

for i in 1..10 do
    for j in 1..10 do
    	sum := sum + 1;
    end for;
end for;

assert(sum = 100);