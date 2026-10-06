#include <stdio.h>

int main()
{
    // declared teh value 
    char str[1001];
    int count[10] = {0};
    int i;

    scanf("%s", str);

    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            count[str[i] - '0']++;
        }
        for (i = 0; i < 10; i++)
        {
            printf("%d", count[i]);
        }
        return 0;
    }
}




// 0100000000  for 1

// 0010000000 for 2

// 0001000000 for 3 

// 1000000000 for 0