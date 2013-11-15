fact = { (i:Integer)
  if true {
    1;
  } else {
    i * fact(i - 1);
  };
};

fact(5); # 120
