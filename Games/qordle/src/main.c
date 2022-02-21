#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/time.h>
#include <libc/string.h>

#define ACTUAL_WORD_LIST "/usr/assets/act.csv"
#define ACCEPTABLE_ANSWER_LIST "/usr/assets/acc.csv"

char *loadFile(char *file);

int acceptableGuess(char *buffer, char guess[128]);

void printHelp();

int main(int argc, char **argv)
{
	if(argc ==2)
	{
		if(! strcmp(argv[1], "--help") || ! strcmp(argv[1], "-h"))
		{
			printHelp();
			return 0;
		}
		else
		{
			printf("use %s -h or %s --help for help", argv[0], argv[0]);
			return 0;
		}
	
	}

	//load word lists into memory as char arrays
        char *buffer1 = loadFile(ACTUAL_WORD_LIST);
        char *buffer2 = loadFile(ACCEPTABLE_ANSWER_LIST);

	//get word of the day and store in WoTD variable.
        time_t seconds = time(NULL);
	char WoTD[6];
        for(int i = 0; i < 6; i++)
        {
                if(buffer1[seconds/86400%2309*6+i] == '\n')
                {
                        WoTD[i] = '\0';
                }
                else
                {
                        WoTD[i] =  buffer1[seconds/86400%2309*6+i];
                }
        }
	free(buffer1);

	//convert all new lines to null terminators
	for(int i = 0; i < 63822; i++)
	{
		if(buffer2[i] == '\n')
		{
			buffer2[i] = '\0';
		}
	}

	//game loop starts here
	char guess[128];
	int guesses = 1;

	while(1)
	{
		printf("[%i] ", guesses);
		gets(guess);

		//check if input is more than 5 chars
		if(strlen(guess) > 5)
		{
			for(int i = 0; i < strlen(guess)-1; i++)
			{
				if( i < 5)
				{
					printf("%c", guess[i]);
				}
				else
				{
					printf("\x1b[41m%c", guess[i]);
				}
			}
			printf("\x1b[0m\n");
		}

		//check if input is not in the acceptable guess list
		else if(acceptableGuess(buffer2, guess) != 1)
		{
			for(int i = 0; i < strlen(guess)-1; i++)
			{
				printf("\x1b[41m%c", guess[i]);
			}
			printf("\x1b[0m\n");
		}
	}			

        free(buffer2);
        return 0;
}

char *loadFile(char *file)
{
        FILE *fp = fopen(file, "rb");
        if(!fp)
        {
                perror("fopen() failed");
                exit(1);
        }

        int length = fseek(fp, 0L, SEEK_END);
        if(length < 0)
        {
                perror("seek failed");
                fclose(fp);
                exit(1);
        }
        fseek(fp, 0, SEEK_SET);

        char *buffer = malloc(length + 2);
        if (!buffer)
        {
                fclose(fp);
		eprintf("Allocation failed.\n");
		exit(1);
        }

        if(fread(buffer, length, 1, fp) != 1)
        {
                perror("fread() failed.");
                fclose(fp);
                exit(1);
        }

        printf("%i\n", length);
        fclose(fp);
        return buffer;
}

int acceptableGuess(char *buffer, char guess[128])
{
	//if guess found in buffer return 0
return 0;
	//if guess not found in buffer return 1
}

void printHelp()
{
	//todo?
	printf("lol it's wordle\n");
}
