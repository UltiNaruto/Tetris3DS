#include <stdio.h>
#define TICKS_PER_MSEC (268.123480)
int	Flips;
int	FPSflips;
double	FPStime;
double	Ticks;
double	FirstFlipTime;
double	ctrticks;
struct HIGH_SCORE {
	char Name[11];
	int Score;
	int Level;
};

vector<HIGH_SCORE> highScores;

char** split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
		/*printf("idx = %d\n", idx);
		printf("count = %d", count);*/
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

char * getProfileName()
{
	Result res;
	u8 * tempProfileName = (u8*)malloc(0x16);
	char * profileName = (char*)malloc(0x16/2);
	int i;
	for(i=0;i<(0x16/2);i++)
		profileName[i] = 0;
	res = CFGU_GetConfigInfoBlk2(0x1C, 0xA0000, tempProfileName);
	for(i=0;i<(0x16/2);i++)
	{
		profileName[i] = (char)((u16*)tempProfileName)[i];
	}
	return profileName;
}

bool compareScore(const HIGH_SCORE &a, const HIGH_SCORE &b)
{
    return a.Score > b.Score;
}

vector<HIGH_SCORE> getHighScores()
{
	return highScores;
}

void saveHighScores(bool def=false)
{
	if(def)
	{
		for(int i=0;i<10;i++)
		{
			HIGH_SCORE newHighScore;
			sprintf(newHighScore.Name,"COM_%d", (i+1));
			newHighScore.Score = POINTS_PER_LEVEL*10 - i * POINTS_PER_LEVEL;
			newHighScore.Level = 13-(i+2);
			highScores.push_back(newHighScore);
		}
		std::sort(highScores.begin(), highScores.end(), compareScore);
		FILE * file = fopen("highScores.bin", "w");
		if(file == NULL)
		{
			aptSetStatusPower(APP_EXITING);
			return;
		}
		for(int i=0;i<10;i++)
		{
			fprintf(file, "%s|", highScores.at(i).Name);
			fprintf(file, "%d|", highScores.at(i).Score);
			fprintf(file, "%d", highScores.at(i).Level);
			fprintf(file, "\n");
		}
		fclose(file);
	}
	else
	{
		std::sort(highScores.begin(), highScores.end(), compareScore);
		FILE * file = fopen("highScores.bin", "w+");
		if(file == NULL)
		{
			aptSetStatusPower(APP_EXITING);
			return;
		}
		for(int i=0;i<10;i++)
		{
			fprintf(file, "%s|", highScores.at(i).Name);
			fprintf(file, "%d|", highScores.at(i).Score);
			fprintf(file, "%d", highScores.at(i).Level);
			fprintf(file, "\n");
		}
		fclose(file);
	}
}

void loadHighScores()
{
	int size = 1024, pos;
    int c;
	char *buffer = (char *)malloc(size);
	FILE * file = fopen("highScores.bin", "r");
	highScores.clear();
	if(file == NULL)
	{
		saveHighScores(true);
	}
	else
	{
		do {
			pos = 0;
			do{
				c = fgetc(file);
				if(c != EOF) buffer[pos++] = (char)c;
				if(pos >= size - 1) {
					size *=2;
					buffer = (char*)realloc(buffer, size);
				}
			}while(c != EOF && c != '\n');
			buffer[pos] = 0;
			if(strlen(buffer) == 0)
				break;
			char ** params = split(buffer, '|');
			HIGH_SCORE newHighScore;
			strcpy(newHighScore.Name, params[0]);
			newHighScore.Score = atol(params[1]);
			newHighScore.Level = atoi(params[2]);
			/*printf("Name : %s\n", newHighScore.Name);
			printf("Score : %d\n", newHighScore.Score);
			printf("Level : %d\n", newHighScore.Level);
			while(1) {}*/
			highScores.push_back(newHighScore);
		} while(c != EOF);
		std::sort(highScores.begin(), highScores.end(), compareScore);
		fclose(file);
	}
}

void addNewHighScore(int score, int level)
{
	if(score > highScores.back().Score)
	{
		highScores.pop_back();
		HIGH_SCORE newHighScore;
		strcpy(newHighScore.Name, getProfileName());
		newHighScore.Score = score;
		newHighScore.Level = level;
		highScores.push_back(newHighScore);
		std::sort(highScores.begin(), highScores.end(), compareScore);
		saveHighScores();
	}
}

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