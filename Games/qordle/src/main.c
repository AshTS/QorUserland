#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/time.h>

#define ACTUAL_WORD_LIST "/usr/assets/act.csv"
#define ACCEPTABLE_ANSWER_LIST "/usr/assets/acc.csv"

char *load(char *file);

int main(int argc, char **argv)
{
        char *buffer1 = load(ACTUAL_WORD_LIST);
        char *buffer2 = load(ACCEPTABLE_ANSWER_LIST);

        time_t seconds = time(NULL);

        printf("Word of the day: ");
        for(int i = 0; i < 5; i++)
        {
                if(buffer1[seconds/86400%2309*6+i] == '\n')
                {
                        printf("N");
                }
                else
                {
                        printf("%c", buffer1[seconds/86400%2309*6+i]);
                }
        }
        printf("\n");

        free(buffer1);
        free(buffer2);
        return 0;
}

char *load(char *file)
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
