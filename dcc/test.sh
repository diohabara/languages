#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./dcc "$input" >tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

start_test() {
  echo "---"
  echo $1
  echo "---"
}

start_test "until step6: plus, minus, cal, div, ()"
try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5 ;"
try 47 "5+6*7;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 10 "-10+20;"

start_test "utntil step7: relational"
try 0 "0==1;"
try 1 "0==0;"
try 0 "1!=1;"
try 1 "1!=88;"
try 0 "1<0;"
try 1 "0<1;"
try 0 "13<=12;"
try 1 "13<=14;"
try 1 "7<=7;"
try 0 "0>1;"
try 1 "1>0;"
try 0 "0>=1;"
try 1 "2>=1;"
try 1 "1>=1;"

start_test "until step9: local varialbles with a single character"
try 3 "a = 3;"
try 22 "b = 5 * 6 - 8;"

start_test "step10: local variables with multiple characters"
try 1 "bar = 1;"
try 11 "bar = 10; bar = foo = 11;"

start_test "step11: return"
try 0 "return 0;"
try 1 "return 1;"

start_test "step12: if, while, for"
echo "~~~if statement~~~"
try 1 "if (1) 1;"
try 1 "if (1) 1; else 0;"
try 0 "if (0) 1; else 0;"
echo "~~~while statement~~~"
try 0 "b = 0; while (0) 1; b;"
try 0 "a = 5; while (a) a = a - 1; a;"
echo "~~~for statement~~~"
try 5 "b = 0; for (a = 0; a < 5; a = a + 1) b = b + 1; b;"
try 55 "a = 0; for (i = 1; i <= 10; i = i + 1) a = a + i; a;"
try 120 "a = 1; for (i = 1; i <= 5; i = i + 1) a = a * i; a;"

start_test "step13: block statement"
try 3 "a = 1; { a = 3; } a;"
try 0 "{ a = 0; }"
try 55 "{ a = 0; for (i = 1; i <= 10; i = i + 1) { a = i; } a; }"

echo OK
