# include<stdio.h>

int main(){

    int n;
    int scores[100][5];
    int total[100];
    int i, j;
    int count = 0;

    printf("Enter the number of students: ");
    scanf("%d", &n);

    for(i = 0; i < n; i++){
        printf("Enter the scores of student %d: \n", i + 1);
        total[i] = 0;
        for(j = 0; j < 5; j++){
            printf("Score %d: ", j + 1);
            scanf("%d", &scores[i][j]);
            total[i] += scores[i][j];
        }
    }

    for (i = 1; i < n; i++){
        if(total[i] > total[0]) {
            count++;
        }
    }

    printf("Number of students with total score greater than Kristen : %d\n", count);

}

