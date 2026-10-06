#include <stdio.h>

#define STACK_SIZE 10


int push(int data[], int top, int value, int size){
    if(top == size - 1){
        printf("Stack Overflow\n");
        return top;
    }
    else{
        top++;
        data[top] = value;
        return top;
    }
}


int pop(int data[], int top){
    if(top == -1){
        printf("Stack Underflow\n");
        return top;
    }
    else{
        printf("Popped value: %d\n", data[top]);
        top--;
        return top;
    }
}


int peek(int data[], int top){
    if(top == -1){
        printf("Stack is empty\n");
        return -1;
    }
    else{
        return data[top];
    }
}

void display(int data[], int top){
    if(top == -1){
        printf("Stack is empty\n");
        return;
    }
    else{
        for(int i=top; i>=0; i--){
            printf("%d ", data[i]);
        }
        printf("\n");
    }
}


int main(void){
    int data[STACK_SIZE];
    int top = -1;

    char str[10] = "HELLO";
    char reversed[10];

    for (int i = 0; str[i] != '\0'; i++) {
        top = push(data, top, str[i], STACK_SIZE);
    }
    
    display(data, top);

    for (int i = 0; i < 5; i++) {
        reversed[i] = peek(data, top);
        top = pop(data, top);
    }

    printf("Reversed string: ");
    for (int i = 0; i < 5; i++) {
        printf("%c", reversed[i]);
    }
    printf("\n");

    return 0;
}

