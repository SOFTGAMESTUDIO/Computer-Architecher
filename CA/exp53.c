#include <stdio.h>

int main(){
    int N = 5, k = 3;
    int arr[N];
    printf("Enter %d elements: ", N);
    for (int i = 0; i < N; i++){
        if (scanf("%d", &arr[i]) != 1){
            printf("Invalid input\n");
            return 1;
        }
    }

    long long topSum = 0;
    long long bottomSum = 0;
    long long maxSum = 0;
    long long currSum = 0;

    for (int i = 0; i < k; i++){
        topSum = topSum + arr[i];
    }

    maxSum = topSum;

    for (int i = 1; i <= k; i++){
        topSum -= arr[k - i];
        bottomSum += arr[N - i];

        currSum = topSum + bottomSum;
        if (currSum > maxSum){
            maxSum = currSum;
        }
    }
    printf("The Larget Sume of K Element : %lld", maxSum);
}