f = { ()
  nested = { ()
    1;
  };
  x = 1;
  y = 1.5;
  x + y + nested();
};

f();
