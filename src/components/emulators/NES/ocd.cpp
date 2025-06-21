extern "C"
{
#include "nofrendo/osd.h"
#include "nofrendo/event.h"
#include "nofrendo/gui.h"
#include "nofrendo/log.h"
#include "nofrendo/nes/nes.h"
#include "nofrendo/nes/nes_pal.h"
#include "nofrendo/nes/nesinput.h"
#include "nofrendo/nofconfig.h"
#include "nofrendo/osd.h"
}

#include <stdio.h>
#include <string.h>

#include "NesManager.h"

#include <components/system_config/system_manager.h>
#include <components/drivers/inputs/user_input.h>
#include <components/drivers/display/displayHal.h>

static void displayTask(void *arg)
{
    bitmap_t *bmp = NULL;
    while (1)
    {
        // xQueueReceive(vidQueue, &bmp, portMAX_DELAY); //skip one frame to drop to 30
        xQueueReceive(vidQueue, &bmp, portMAX_DELAY);
        display_HAL_NES_frame((const uint8_t *)bmp->line[0]);
    }
}

/* get info */
static char fb[1]; // dummy
bitmap_t *myBitmap;

/* initialise video */
int8 btn_ss; // Variable to save button state save selected option
static int init(int width, int height)
{
    btn_ss = sys_manager.system_get_config(SYS_STATE_SAV_BTN); // Check if we want state save/load buttons available
    return 0;
}

static void shutdown(void)
{
}

/* set a video mode */
static int set_mode(int width, int height)
{
    return 0;
}

static void set_palette(rgb_t *pal)
{
    uint16 c;
    int i;
    for (i = 0; i < 256; i++)
    {
        c = (pal[i].b >> 3) + ((pal[i].g >> 2) << 5) + ((pal[i].r >> 3) << 11);
        myPalette[i] = (c >> 8) | ((c & 0xff) << 8);
        // myPalette[i] = c;
    }
}

/* clear all frames to a particular color */
static void clear(uint8 color)
{
    // SDL_FillRect(mySurface, 0, color);
    // display_clear(); //TODO:Modified
    // display_HAL_clear();
    clear_screen();
}

/* acquire the directbuffer for writing */
static bitmap_t *lock_write(void)
{
    // SDL_LockSurface(mySurface);
    myBitmap = bmp_createhw((uint8 *)fb, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, NES_SCREEN_WIDTH * 2); //
    return myBitmap;
}
/* release the resource */
static void free_write(int num_dirties, rect_t *dirty_rects)
{
    bmp_destroy(&myBitmap);
}

static void custom_blit(bitmap_t *bmp, int num_dirties, rect_t *dirty_rects)
{
    xQueueSend(nofrendo_vidQueue, &bmp, 0);
    // do_audio_frame();
}

viddriver_t sdlDriver =
    {
        "Simple DirectMedia Layer", /* name */
        init,                       /* init */
        shutdown,                   /* shutdown */
        set_mode,                   /* set_mode */
        set_palette,                /* set_palette */
        clear,                      /* clear */
        lock_write,                 /* lock_write */
        free_write,                 /* free_write */
        custom_blit,                /* custom_blit */
        false                       /* invalidate flag */
};

void osd_getvideoinfo(vidinfo_t *info)
{
    info->default_width = NES_SCREEN_WIDTH;
    info->default_height = NES_SCREEN_HEIGHT;
    info->driver = &sdlDriver;
}

/*

    |||||||||||||||||||||||||||||||||||||||||||||||||||||||||
    ||  R1           |------------------------|        R2  ||
    ||               | 240 320                |            ||
    ||               | Display                |            ||
    ||      up       |                        |  B     A   ||
    || left    right |                        |            ||
    ||     down      |________________________|  X     Y   ||
    ||              select    menu    start                ||
    |||||||||||||||||||||||||||||||||||||||||||||||||||||||||

*/

void osd_getinput(void)
{
    uint16_t b = UserInput::input_read();

    const int ev[16] = {
        event_joypad1_up,    // up
        event_joypad1_down,  // down
        event_joypad1_right, // right
        event_joypad1_left,  // left
        0,
        (btn_ss & 1) ? event_state_save : 0, // menu
        event_joypad1_start,                 // start
        event_joypad1_select,                // select
        0,
        event_joypad1_b, // B
        event_joypad1_a, // A
        0,               //(btn_ss & 1) ? event_state_save : 0, // select on physical keyboard
        0,               //(btn_ss & 1) ? event_state_load : 0, // start on physical keyboard
        0,
        0,
        0
        ///
    };

    static int oldb = 0xffff;
    int chg = b ^ oldb;
    int x;
    oldb = b;
    event_t evh;

    for (x = 0; x < 16; x++)
    {
        if (chg & 1)
        {
            evh = event_get(ev[x]);
            if (evh)
                evh((b & 1) ? INP_STATE_BREAK : INP_STATE_MAKE);
        }
        chg >>= 1;
        b >>= 1;
    }
}

void osd_getmouse(int *x, int *y, int *button)
{
}

/* init / shutdown */
static int logprint(const char *string)
{
    return printf("%s", string);
}

int osd_init()
{
    nofrendo_log_chain_logfunc(logprint);

    // if (osd_init_sound())
    //     return -1;

    // display_init(); //TODO: Modified
    printf("osd_init\r\n");

    return 0;
}

void osd_shutdown()
{
    // osd_stopsound();
}

char configfilename[] = "na";
int osd_main(int argc, char *argv[])
{
    config.filename = configfilename;
    return main_loop(argv[0], system_autodetect);
}

// Define osd_setsound
void osd_setsound(void (*playfunc)(void *buffer, int size))
{
    // Implementation of osd_setsound
}

// Define osd_getsoundinfo
void osd_getsoundinfo(struct osd_soundinfo *info)
{
    // Implementation of osd_getsoundinfo
}

// Seemingly, this will be called only once. Should call func with a freq of frequency,
int osd_installtimer(int frequency, void *func, int funcsize, void *counter, int countersize)
{
    nofrendo_log_printf("Timer install, configTICK_RATE_HZ=%d, freq=%d\n", configTICK_RATE_HZ, frequency);
    timer = xTimerCreate("nes", configTICK_RATE_HZ / frequency, pdTRUE, NULL, (TimerCallbackFunction_t)func);
    xTimerStart(timer, 0);
    return 0;
}

/* filename manipulation */
void osd_fullname(char *fullname, const char *shortname)
{
    strncpy(fullname, shortname, PATH_MAX);
}

/* This gives filenames for storage of saves */
char *osd_newextension(char *string, char *ext)
{
    // dirty: assume both extensions is 3 characters
    size_t l = strlen(string);
    string[l - 3] = ext[1];
    string[l - 2] = ext[2];
    string[l - 1] = ext[3];

    return string;
}

/* This gives filenames for storage of PCX snapshots */
int osd_makesnapname(char *filename, int len)
{
    return -1;
}