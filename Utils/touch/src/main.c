/* 
	qor touch -- create files
	Copyright (C) 2022 Ethan Hunt
*/

#include <libc/stdio.h>

#define PROGRAM_NAME "touch"
#define AUTHOR "Ethan Hunt"

int main(int argc, char *argv[])
{
        if(argc == 2)
        {
                FILE *fp = fopen(argv[1], "a+");
		if (fp == NULL) {
			perror("fopen() failed");
		} 
		else
		{
			fclose(fp);
		}
                return 0;
        }
        else
        {
                printf("Usage: %s <filename>\n", argv[0]);
                return 1;
        }
}
