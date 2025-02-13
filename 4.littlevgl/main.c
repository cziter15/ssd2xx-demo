#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_demos/lv_demo.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#define DISP_BUF_SIZE (256 * 1024)

int main(int argc, char *argv[])
{
    int demo_id = 10;
    int hor_res = 800;
    int ver_res = 480;

    if (argc >= 3)
    {
        hor_res = atoi(argv[1]);
        ver_res = atoi(argv[2]);
    }
    else if (argc >= 4)
    {
        demo_id = atoi(argv[3]);
    }
    else
    {
        fprintf(stderr,"Warring: Usage: %s [w,h] [deme_id]\r\n"
               "deme_id: 0 1 2 3 \r\n"
               "eg: %s 1024 600 3 ",
               argv[0], argv[0]);
    }

    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = hor_res;
    disp_drv.ver_res = ver_res;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    evdev_init();
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = (void (*)(struct _lv_indev_drv_t *, lv_indev_data_t *))evdev_read; // defined in lv_drivers/indev/evdev.h
    lv_indev_t *lv_indev = lv_indev_drv_register(&indev_drv);
    if (!lv_indev)
    {
        printf("lv_indev rregister error %d \r\n", __LINE__);
        return 0;
    }

    /*Create a Demo*/
    switch (demo_id)
    {
    case 0:
        lv_demo_music();
        break;
    case 1:
        lv_demo_stress();
        break;
    case 2:
        lv_demo_keypad_encoder();
        break;
    default:
        lv_demo_widgets();
        break;
    }

    /*Handle LitlevGL tasks (tickless mode)*/
    while (1)
    {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
