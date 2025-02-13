#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <mi_common.h>
#include <mi_sys_datatype.h>
#include <mi_sys.h>
#include "sstarFb.h"
#include "jpeg.h"

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;

//Start of frame buffer mem
static char *frameBuffer = NULL;

#define LOGO_SUFFIX_JPEG ".jpg"

static void CompileFormat(PixelFormat *format, int bpp,
                          MI_U32 Rmask, MI_U32 Gmask, MI_U32 Bmask, MI_U32 Amask)
{
    MI_U32 mask;

    format->alpha = 0xFF;

    switch (bpp)
    {
    case 1:
    case 4:
    case 8:
    {
        assert(0);
        printf("%s %d unsupport\n", __FUNCTION__, __LINE__);
    }

    default:
        format->Rshift = 0;
        format->Rloss = 8;

        if (Rmask)
        {
            for (mask = Rmask; !(mask & 0x01); mask >>= 1)
                ++format->Rshift;

            for (; (mask & 0x01); mask >>= 1)
                --format->Rloss;
        }

        format->Gshift = 0;
        format->Gloss = 8;

        if (Gmask)
        {
            for (mask = Gmask; !(mask & 0x01); mask >>= 1)
                ++format->Gshift;

            for (; (mask & 0x01); mask >>= 1)
                --format->Gloss;
        }

        format->Bshift = 0;
        format->Bloss = 8;

        if (Bmask)
        {
            for (mask = Bmask; !(mask & 0x01); mask >>= 1)
                ++format->Bshift;

            for (; (mask & 0x01); mask >>= 1)
                --format->Bloss;
        }

        format->Ashift = 0;
        format->Aloss = 8;

        if (Amask)
        {
            for (mask = Amask; !(mask & 0x01); mask >>= 1)
                ++format->Ashift;

            for (; (mask & 0x01); mask >>= 1)
                --format->Aloss;
        }

        format->Rmask = Rmask;
        format->Gmask = Gmask;
        format->Bmask = Bmask;
        format->Amask = Amask;
        break;
    }

    /* Calculate some standard bitmasks, if necessary
     * Note:  This could conflict with an alpha mask, if given.
     */
    if ((bpp > 8) && !format->Rmask && !format->Gmask && !format->Bmask)
    {
        /* R-G-B */
        if (bpp > 24)
            bpp = 24;

        format->Rloss = 8 - (bpp / 3);
        format->Gloss = 8 - (bpp / 3) - (bpp % 3);
        format->Bloss = 8 - (bpp / 3);
        format->Rshift = ((bpp / 3) + (bpp % 3)) + (bpp / 3);
        format->Gshift = (bpp / 3);
        format->Bshift = 0;
        format->Rmask = ((0xFF >> format->Rloss) << format->Rshift);
        format->Gmask = ((0xFF >> format->Gloss) << format->Gshift);
        format->Bmask = ((0xFF >> format->Bloss) << format->Bshift);
    }
}

static void syncFormat(BITMAP *bmp, struct fb_var_screeninfo *vinfo)
{
    MI_U32 Rmask;
    MI_U32 Gmask;
    MI_U32 Bmask;
    MI_U32 Amask;
    int i;

    Rmask = 0;

    for (i = 0; i < vinfo->red.length; ++i)
    {
        Rmask <<= 1;
        Rmask |= (0x00000001 << vinfo->red.offset);
    }

    Gmask = 0;

    for (i = 0; i < vinfo->green.length; ++i)
    {
        Gmask <<= 1;
        Gmask |= (0x00000001 << vinfo->green.offset);
    }

    Bmask = 0;

    for (i = 0; i < vinfo->blue.length; ++i)
    {
        Bmask <<= 1;
        Bmask |= (0x00000001 << vinfo->blue.offset);
    }

    Amask = 0;

    for (i = 0; i < vinfo->transp.length; ++i)
    {
        Amask <<= 1;
        Amask |= (0x00000001 << vinfo->transp.offset);
    }

    CompileFormat(&bmp->pxFmt, bmp->bmBitsPerPixel, Rmask, Gmask, Bmask, Amask);
}

int main(int argc, char **argv)
{
    const char *devfile = "/dev/fb0";
    long int screensize = 0;
    int fbFd = 0;
    FILE *fp;
    BITMAP fb;
    char *sLogoPath = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "eg: %s ./res/detail.jpg\r\n",argv[0]);
        exit(1);
    }

    sLogoPath = argv[1];

    /* Open the file for reading and writing */
    fbFd = open(devfile, O_RDWR);

    if (fbFd == -1)
    {
        perror("Error: cannot open framebuffer device");
        exit(1);
    }

    int show = 0;

    if (ioctl(fbFd, FBIOSET_SHOW, &show) < 0)
    {
        return (0);
    }

    //get fb_fix_screeninfo
    if (ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error reading fixed information");
        exit(2);
    }

    //get fb_var_screeninfo
    if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error reading variable information");
        exit(3);
    }

    fb.bmPhyAddr = finfo.smem_start;
    fb.bmHeight = vinfo.yres;
    fb.bmWidth = vinfo.xres;
    fb.bmPitch = finfo.line_length;
    fb.bmBitsPerPixel = vinfo.bits_per_pixel;
    fb.bmBytesPerPixel = vinfo.bits_per_pixel / 8;
    syncFormat(&fb, &vinfo);

    screensize = finfo.smem_len;
    frameBuffer = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);

    if (frameBuffer == MAP_FAILED)
    {
        perror("Error: Failed to map framebuffer device to memory");
        exit(4);
    }

    if ((fp = fopen(sLogoPath, "r")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", sLogoPath);
        return 0;
    }

    if (strstr(sLogoPath, LOGO_SUFFIX_JPEG) != NULL)
    {
        load_logo_jpeg(frameBuffer, &fb, fp);
        goto SHOW;
    }
    else
    {
        fprintf(stderr, "logo format not support");
        fclose(fp);
        return 1;
    }
SHOW:

    //Pandisplay
    if (ioctl(fbFd, FBIOPAN_DISPLAY, &vinfo) == -1)
    {
        perror("Error: failed to FBIOPAN_DISPLAY");
        exit(1);
    }

    //unmap buffer
    munmap(frameBuffer, screensize);

    fclose(fp);
    close(fbFd);
    return 0;
}
