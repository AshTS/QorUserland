#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/time.h>
#include <libc/stdlib.h>

int main(int argc, char **argv)
{

	FILE *fp = fopen("/usr/assets/acc.csv", "rb");
	if (!fp)
	{
    		perror("fopen() failed.");
    		return(1);
	}

	int length = fseek(fp , 0L , SEEK_END);
	if (length < 0)
	{
    		perror("seek failed");
    		fclose(fp);
    		return(1);
	}
	fseek(fp, 0, SEEK_SET);

	char *buffer = malloc(length + 1);
	if (!buffer)
	{
    		fclose(fp);
    		eprintf("Allocation Failed\n");
    		return(1);
	}

	if (fread(buffer, length, 1, fp) != 1)
	{
		perror("seek failed");
    		fclose(fp);
    		return(1);
	}

	// This is where you do your thing with the buffer

	fclose(fp);
	free(buffer);
	return 0;
}	
