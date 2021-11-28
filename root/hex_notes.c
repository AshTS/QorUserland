size_t strlen(char* s)
{
    size_t len = 0;

    while (*s)
    {
        len++;
        s++;
    }

    return len;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        return 1;
    }

    char* fname = argv[1];

    size_t len = strlen(fname);

    write(1, fname, len);

    write(1, "\n", 1);
}

void hex_dump(void* buffer, size_t length)
{
    write_str("       0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F\n");
    
    size_t row = 0;

    while (row * 16 < length)
    {
        write_space();
        write_hex(row >> 8);
        write_hex(row);
        write_space();

        size_t i = 0;

        while (i < 16)
        {
            char* this = buffer + row * 16 + i;
            write_hex(*this);

            write_space();
            
            if (i == 7)
            {
                write_space();
            }

            i++;
        }

        write_newline();

        row++;
    }
}