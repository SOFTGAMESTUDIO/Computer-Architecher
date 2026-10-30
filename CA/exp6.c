#include <stdio.h>
static int st[100000];
int main()
{
    int t;
    scanf("%d", &t);

    while (t--)
    {
        int n, k, i, x, top = 0, rem;
        scanf("%d %d", &n, &k);
        rem = k;

        for (i = 0; i < n; i++)
        {
            scanf("%d", &x);

            /* delete friend if popularity is less than the next friend */ while (rem > 0 && top > 0 && st[top - 1] < x)
            {
                top--;
                rem--;
            }
            st[top++] = x;
        }

        /* if deletions are left, delete from the end */ top -= rem;

        for (i = 0; i < top; i++)
        {
            printf("%d", st[i]);
            if (i < top - 1)
                printf(" ");
        }
        printf("\n");
    }

    return 0;
}
