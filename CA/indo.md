# <center> Experiment 2 </center>

# Variadic Function 

 
### Key points 
* (...) ellipsis 
* At lees 1 no of perimetter befour the dot's
* enter valid syntax of varidic fun.

### Steps: 
step 1: va_list : get an empty "holder" va_list args;
step 2: va_start : Start unpacking  va_start(args, count);
step 3: va_args :  tacke out one value at a time int number - va_args(args, int); //read next number 
step 4: va_end : to end of the program and expty arguments - va_end(args);

 <pre style=" background-color: black; color: white; border: 2px solid green; border-radius: 25px;">
#include <stdio.h>
#include <stdarg.h>

// sum 
int Is_sum(int count, ...){
    va_list args;

    va_start(args, count);

    int total = 0;
    for (int i = 0; i < count; i++){
      total = total + va_arg(args, int);
    }
    
    va_end(args);
    return total;

}

// min 
int Is_Min(int count, ...){
    va_list args;
    va_start(args, count);

    int small = va_arg(args, int);
    for(int i = 1; i < count; i++){
        int current = va_arg(args, int);
        if(current < small){
            small = current;
        }
    }

    va_end(args);
    return small;
}

// max
int Is_max(int count, ...){
    va_list args;
    va_start(args, count);

    int max = va_arg(args, int);
    for(int i = 1; i < count; i++){
        int current = va_arg(args, int);
        if(current > max){
            max = current;
        }
    }

    va_end(args);
    return max;
}


// main fun.

int main(){
    printf(" The Sum of the number is 10, 20, 30 : \n ");
    int add = Is_sum(3, 10, 20, 30);
    printf("The Addition of Numbers : %d \n ", add);
    printf("The Sum of the number is 10, 20, 30 : \n ");
    int minimum = Is_Min(3, 30, 10, 20);
    printf("The Miniimum value : %d \n ", minimum);
    printf("The Sum of the number is 10, 20, 30 : \n ");
    int maximum = Is_max(3, 20, 30, 10);
    printf("The Maximum value : %d \n ", maximum);

    return 0;
}


</pre>