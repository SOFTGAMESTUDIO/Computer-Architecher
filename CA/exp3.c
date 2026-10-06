#include <stdio.h>
#include <stdlib.h>


void SetValue(int *a, int *b) {
      int org_a = *a;
      int org_b = *b;

      *a = org_a + org_b;
      *b = abs(org_a - org_b);
}

int main() {
    int x ,y;
    printf("Enter two integers: \n");

    scanf("%d", &x);
    scanf("%d", &y);
    

    SetValue(&x, &y); 

    printf("After SetValue with pointer :\n");
    printf("Value of x: %d\n", x);
    printf("Value of y: %d\n", y);
    return 0;
    
}