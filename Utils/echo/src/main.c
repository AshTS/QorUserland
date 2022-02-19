/*
	qor echo -- echos strings back
	Copyright (C) 2021-2022 Ethan Hunt
*/
#include <libc/stdio.h>

#define PROGRAM_NAME "echo"
#define AUTHOR \
	proper_name ("Ethan Hunt"), \
	proper_name ("Carter Plasek") 

int main (int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		printf("%s", argv[i]);

		if (i < argc - 1)
		{
			printf(" ");
		}
	}
	printf("\n");
	return 0;
}
