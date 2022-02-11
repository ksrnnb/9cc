#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s

    cd func
    cc -c func.c
    cd ..
    cc -static -o tmp tmp.s func/func.o

    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

# comment
assert 100 'int main() {
    char *p;
    p = "abc//def";
    return p[5];
}'

assert 10 'int main() {
    // return 1;
    /*
       return 5;
    */
    return 10;
}'

# string
# ASCII code: a = 97, b = 98, ...
assert 100 'int main() {
    char *a;
    a = "abcd";
    return a[3];
}'

assert 97 'int main() {
    char *a;
    a = "abcd";
    return a[0];
}'

# char
assert 3 "int main() {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}"

# sizeof (char)
assert 1 "int main() {
    char x;
    return sizeof(x);
}"

assert 8 "int main() {
    char *x;
    return sizeof(x);
}"

assert 1 "int main() {
    char *x;
    return sizeof(*x);
}"

# global variables
assert 1 "
int a;
int b[10];
int main() {
    a = 1;
    return a;
}
"

assert 1 "
int a;
int b[10];
int main() {
    return 1;
}
"

# array
assert 3 "int main() {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    int *p;
    p = a;
    return p[0] + p[1];
}"

# array2pointer
assert 3 "int main() {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}"

# array
assert 0 "int main() {
    int x[10];
    return 0;
}"

# sizeof
assert 4 "int main() {
    int x;
    return sizeof(x);
}"

assert 8 "int main() {
    int *y;
    return sizeof(y);
}"

assert 4 "int main() {
    int x;
    return sizeof(x + 3);
}"

assert 8 "int main() {
    int *y;
    return sizeof(y + 3);
}"

assert 4 "int main() {
    int *y;
    return sizeof(*y);
}"

assert 4 "int main() {
    return sizeof(1);
}"

assert 4 "int main() {
    return sizeof(sizeof(1));
}"

# pointer add/sub
assert 2 "int main() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 3;
    int *r;
    r = q - 2;
    return *r;
}"

assert 4 "int main() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 2;
    return *q;
}"

assert 8 "int main() {
    int *p;
    alloc4(&p, 1, 2, 4, 8);
    int *q;
    q = p + 3;
    return *q;
}"

# pointer type
assert 3 "int main() {
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}"

# int type
assert 3 "int main() {
    int x;
    x = 3;
    return x;
}"

assert 3 "int main() {
    int x;
    int *y;
    x = 3;
    y = &x;
    return *y;
}"

assert 3 "int main() {
    int x;
    int y;
    int *z;
    x = 3;
    y = 5;
    z = &y + 4;
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