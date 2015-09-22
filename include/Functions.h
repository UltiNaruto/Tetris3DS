#include <stdio.h>
#define TICKS_PER_MSEC (268.123480)
int	Flips;
int	FPSflips;
double	FPStime;
double	Ticks;
double	FirstFlipTime;
double	ctrticks;

double srv_get_microseconds()
{
	double ticks;
	
	ctrticks = svcGetSystemTick();
	ticks = ctrticks/TICKS_PER_MSEC;
	return(ticks);
}

double	CurrentFPS = (FPSflips * 1000000.0)/(srv_get_microseconds() - FPStime);

void ClearScreenBuffer(gfxScreen_t screen)
{
	int i;
	u16 * fb;
	int width = (screen == GFX_TOP ? 400 : 320);
	fb= (u16*) gfxGetFramebuffer(screen, GFX_LEFT, NULL, NULL);
	for (i=0; i<width*240;i++) fb[i] = 0;
}

// Big thanks to Tjessx from gbatemp.net for those 2 functions
// It helped me a lot to draw blocks

void color_pixel(int x, int y, u8 r, u8 g, u8 b, u8* screen) {
	int HEIGHT = 240;
    y = HEIGHT - y;
    int hulp = x;
    x = y;
    y = hulp;
   
    u32 v=(x + y * HEIGHT) * 3;
   
    if((v < 288000 && v > 0) || (v < 230400 && v > 0)) {
        screen[v] = b;
        screen[v + 1] = g;
        screen[v + 2] = r;
    }
}

void drawImage(gfxScreen_t screen, const void * image, int width, int height, int x, int y)
{
    int len = width * height;
   
    u8* fb = gfxGetFramebuffer(screen, GFX_LEFT, NULL, NULL);
    u8* toFB = (u8*)image;
   
    int i, j, v;
    for(i = 0; i < height; i++) {
        for(j = 0; j < width; j++) {
            v = (height * i + width) * 3;
            color_pixel(x + j, y + i, toFB[v], toFB[v++], toFB[v++], fb);
        }
    }
}

void drawBackground(gfxScreen_t screen, const void * image, int len)
{
    u8* fb = gfxGetFramebuffer(screen, GFX_LEFT, NULL, NULL);
    u8* toFB = (u8*)image;
    for(int i=0;i<len;i++)
    {
        fb[i] = toFB[i];
    }
}

// Thanks to nop90 for his flip function which updates the FPS count very accurately

void srv_Flip()
{
	double Now = srv_get_microseconds();
	
	if (Ticks == 0.0)	Ticks = Now;
	if (Flips++ == 0)	FirstFlipTime = Now;
	if (FPStime == 0.0)	FPStime = Now;

	++FPSflips;

	if (Now - FPStime > 1000000.0)	/* update FPS every 4 seconds or ... */
	{	
		CurrentFPS = (FPSflips * 1000000.0)/(Now - FPStime);
		FPStime = Now;
		FPSflips = 0;
		consoleClear();
	}
	else if (Now - FPStime <= 0.0)	/* ... if there was a time quake */
	{
		CurrentFPS = 0.0;
		FPStime = Now;
		FPSflips = 0;
	}
}