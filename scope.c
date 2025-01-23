#include <stdio.h>

int *foo()
{
    int x = 1;
    return &x; // returning the address of a local variable
}

int main()
{
    int *p = foo(); // p points to a local variable that gets destroyed after foo() returns
    printf("%d\n", *p);
    *p = 2; // undefined behavior
    printf("%d\n", *p);
    return 0;
}