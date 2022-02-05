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

# int type
assert 3 "int main() {
    int x;
    x = 3;
    return x;
}"

assert 3 "int main() {
    int x;
    int y;
    x = 3;
    y = &x;
    return *y;
}"

assert 3 "int main() {
    int x;
    int y;
    int z;
    x = 3;
    y = 5;
    z = &y + 8;
    return *z;
}"

assert 0 "int main() {
    return 0;
}"

assert 2 "int main() {
    return func();
}

int func() {
    return 2;
}
"

assert 3 "int main() {
    return func(1, 2);
}

int func(int x, int y) {
    return x + y;
}
"

assert 5 "int main() {
    return func(1, 2, 3);
}

int func(int x, int y, int z) {
    return y + z;
}
"

# using local variable
assert 120 "int main() {
    int a;
    a = 5;
    return fractal(a);
}

int fractal(int n) {
    if (n <= 1) {
        return 1;
    }

    return n * fractal(n - 1);
}"

assert 42 "int main() {return 42;}"

assert 21 "int main() { return 5+20-4;}"
assert 41 "int main() { return 12 + 34 - 5;}"

assert 47 "int main() { return 5+6*7;}"
assert 15 "int main() { return 5*(9-6);}"
assert 4 "int main() { return (3+5)/2;}"

assert 10 "int main() { return -10+20;}"
assert 5 "int main() { return +20-15;}"

assert 0 "int main() { return 1>2;}"
assert 0 "int main() { return 2>2;}"
assert 1 "int main() { return 2>1;}"
assert 1 "int main() { return 2>=1;}"
assert 1 "int main() { return 2>=2;}"
assert 0 "int main() { return 2>=3;}"
assert 1 "int main() { return 3<4;}"
assert 0 "int main() { return 3<3;}"
assert 1 "int main() { return 3<=3;}"
assert 0 "int main() { return 3<=2;}"
assert 1 "int main() { return 2==2;}"
assert 0 "int main() { return 1==2;}"
assert 1 "int main() { return 1!=2;}"
assert 0 "int main() { return 2!=2;}"

assert 1 "int main() { int foo; foo=4; return foo==4;}"
assert 1 "int main() { int bar; bar=4; return bar!=3;}"
assert 0 "int main() { int a; int b; a=4; b=3; return a==b;}"
assert 1 "int main() { int a; int b; a=4; b=4; return a==b;}"

assert 5 "int main() { int a; a = 1; if (a == 1) return 5;}"
assert 5 "int main() { int a; a = 1; if (a == 1) return 5; else return 0;}"
assert 0 "int main() { int a; a = 10; if (a == 1) return 5; else return 0;}"

assert 10 "int main() { int i; i = 1; while(i < 10) i = i + 1; return i;}"
assert 1 "int main() { int i; i = 1; while(i > 10) i = i + 1; return i;}"

assert 90 "int main() { int i; int j; j = 0; for (i = 1; i < 10; i = i + 1) j = j + i * 2; return j;}"

assert 4 "int main() { int a; int b; a = 1; if (a == 1) { b = 3; a = a + b; } return a;}"

# function
assert 1 "int main() { return foo();}"
assert 10 "int main() { return bar(4, 6);}"
assert 15 "int main() { return bar3(4, 5, 6);}"

echo OK