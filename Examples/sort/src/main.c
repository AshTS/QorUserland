#include <libc/stdio.h>
#include <libc/stdlib.h>

int cmp(const void* a, const void* b)
{
    return *(int*)a - *(int*)b;
}

int main(int argc, char** argv)
{
    int data[11] = {8, 2, 5, 6, 9, 1, 0, 2, 3, 7, 4};

    printf("Before:\n[");

    for (int i = 0; i < 11; i++)
    {
        if (i)
        {
            printf(", ");
        }

        printf("%i", data[i]);
    }

    printf("]\n");

    qsort(data, 11, sizeof(int), cmp);

    printf("After:\n[");

    for (int i = 0; i < 11; i++)
    {
        if (i)
        {
            printf(", ");
        }

        printf("%i", data[i]);
    }

    printf("]\n");

    return 0;
}