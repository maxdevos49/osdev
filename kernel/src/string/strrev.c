#include <stddef.h>

#include "utility.h"

void strrev(char *str)
{
    if (str == NULL)
    {
        return;
    }

    int i = 0;
    int j = strlen(str) - 1;

    while (i < j)
    {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
        i++;
        j--;
    }
}