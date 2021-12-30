#include <libc/stdio.h>

#include <graphics.h>
#include <libimg.h>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("img requires atleast one argument\n");
        return 1;
    }
    
    struct pixel_buffer data;

    if (load_image(argv[1], &data))
    {
        printf("Image load failed!\n");
    }

    blit(&data, 320 - data.width / 2, 240 - data.height / 2);

    return 0;
}


