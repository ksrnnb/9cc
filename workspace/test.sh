#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s

    cd func
    cc -c func.c
    cd ..
    cc -o tmp tmp.s func/func.o

    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"

assert 21 "5+20-4;"
assert 41 "12 + 34 - 5;"

assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"

assert 10 "-10+20;"
assert 5 "+20-15;"

assert 0 "1>2;"
assert 0 "2>2;"
assert 1 "2>1;"
assert 1 "2>=1;"
assert 1 "2>=2;"
assert 0 "2>=3;"
assert 1 "3<4;"
assert 0 "3<3;"
assert 1 "3<=3;"
assert 0 "3<=2;"
assert 1 "2==2;"
assert 0 "1==2;"
assert 1 "1!=2;"
assert 0 "2!=2;"

assert 1 "foo=4; foo==4;"
assert 1 "bar=4; bar!=3;"
assert 0 "a=4; b=3; a==b;"
assert 1 "a=4; b=4; a==b;"

assert 5 "return 5;"

assert 5 "a = 1; if (a == 1) 5;"
assert 5 "a = 1; if (a == 1) 5; else 0;"
assert 0 "a = 10; if (a == 1) 5; else 0;"

assert 10 "i = 1; while(i < 10) i = i + 1; return i;"
assert 1 "i = 1; while(i > 10) i = i + 1; return i;"

assert 90 "j = 0; for (i = 1; i < 10; i = i + 1) j = j + i * 2; return j;"

assert 4 "a = 1; if (a == 1) { b = 3; a = a + b; } return a;"

# function
assert 0 "foo();"
assert 0 "bar(4, 6);"

echo OK