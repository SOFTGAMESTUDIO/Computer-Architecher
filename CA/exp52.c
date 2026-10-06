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

void is_underflow(int top){

    if (top == -1){
        printf("Stack Underflow\n");
    }
    else{
        printf("Stack is not empty\n");
    }
}

void is_Overflow(int top, int size){

    if (top == size - 1){
        printf("Stack Overflow\n");
    }
    else{
        printf("Stack is not empty\n");
    }
}

int main(void){
    int data[STACK_SIZE];
    int top = -1;
    int choice;
    int value;

    do{
        printf("\n1. Push\n2. Pop\n3. Peek\n4. Display\n5. Is Underflow\n6. Is Overflow\n7. Exit\n");
        printf("Enter your choice: ");

        if(scanf("%d", &choice) != 1){
            printf("Invalid input\n");
            return 1;
        }

        switch(choice){
            case 1:
                printf("Enter value: ");
                if(scanf("%d", &value) != 1){
                    printf("Invalid input\n");
                    return 1;
                }
                top = push(data, top, value, STACK_SIZE);
                break;
            case 2:
                top = pop(data, top);
                break;
            case 3:
                value = peek(data, top);
                if(value != -1){
                    printf("Top value: %d\n", value);
                }
                break;
            case 4:
                display(data, top);
                break;
            case 5:
                is_underflow(top);
                break;
            case 6:
                is_Overflow(top, STACK_SIZE);
                break;
            case 7:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice\n");
        }
        
    }while(choice != 7);

    return 0;
}

