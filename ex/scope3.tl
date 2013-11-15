a = 1;
x = { () a = 2; a + 2; };
y = { (a:Integer) a + 5; };
a; # 1
x(); # 4
y(a); # 6
a + x() + y(a);
