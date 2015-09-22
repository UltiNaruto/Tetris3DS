#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <3ds/console.h>
#include <vector>
#include <math.h>
#include <malloc.h>
#include <inttypes.h>
#include "background_bgr.h"
#include "block_red_bgr.h"
#include "block_purple_bgr.h"
#include "block_grey_bgr.h"
#include "block_blue_bgr.h"
#include "block_green_bgr.h"
#include "block_black_bgr.h"
#include "block_yellow_bgr.h"
#include "Defines.h" // Our defines header
#include "Enums.h"   // Our enums header
#include "Functions.h"
#include "cBlock.h"
using namespace std;
static u32 brightnessSub;
Handle threadHandle, threadRequest;
volatile bool threadExit = false;

void Init();
void Shutdown();
void Game();
void GameWon();
void GameLost();
void HandleGameInput();
void HandleWinLoseInput();
bool CheckEntityCollisions(cSquare* square, Direction dir);
bool CheckWallCollisions(cSquare* square, Direction dir);
bool CheckEntityCollisions(cBlock* block, Direction dir);
bool CheckWallCollisions(cBlock* block, Direction dir);
bool CheckRotationCollisions(cBlock* block); 
void CheckWin();
void CheckLoss();
void HandleBottomCollision();
void ChangeFocusBlock();
int CheckCompletedLines();

void memset(unsigned char * s, int c, size_t len)
{
	for(int i=0;i<len;i++)
		s[i] = c;
}

void disableBacklight() {
	u32 off = 0;
	GSPGPU_ReadHWRegs(NULL, REG_LCDBACKLIGHTSUB, &brightnessSub, 4);
	GSPGPU_WriteHWRegs(NULL, REG_LCDBACKLIGHTSUB, &off, 4);
}

int				   g_Timer;             // Our timer is just an integer
cBlock*			   g_FocusBlock = NULL; // The block the player is controlling
cBlock*			   g_NextBlock  = NULL; // The next block to be the focus block
vector<cSquare*>   g_OldSquares;        // The squares that no longer form the focus block
int				   g_Score = 0;         // Players current score
int				   g_Level = 1;         // Current level player is on
int				   g_FocusBlockSpeed = INITIAL_SPEED; // Speed of the focus block
bool quit = false;

int main(int argc, char **argv)
{
	aptInit();
	gfxInitDefault(); 
	gfxSet3D(false);
	ClearScreenBuffer(GFX_TOP);
	consoleInit(GFX_BOTTOM, NULL);
	Init();
	
	while (aptMainLoop())
	{
		if(quit)
			break;
		Game();
	}

	Shutdown();
	gfxExit();
	aptExit();
	return 0;
}

void Init()
{
	srand(time(NULL));
	
	g_Score = 0;         // Players current score
	g_Level = 1;         // Current level player is on
	g_FocusBlockSpeed = INITIAL_SPEED; // Speed of the focus block
	consoleClear();
	g_FocusBlock = new cBlock( BLOCK_START_X, BLOCK_START_Y, (BlockType)(rand()%7) );
	g_NextBlock  = new cBlock( NEXT_BLOCK_CIRCLE_X, NEXT_BLOCK_CIRCLE_Y, (BlockType)(rand()%7) );
}

void Game()
{
	static int force_down_counter = 0;
	static int slide_counter = SLIDE_TIME;
	
	HandleGameInput();
	
	force_down_counter++;

	if (force_down_counter >= g_FocusBlockSpeed)
	{
		// Always check for collisions before moving anything //
		if ( !CheckWallCollisions(g_FocusBlock, DOWN) && !CheckEntityCollisions(g_FocusBlock, DOWN) )
		{
			g_FocusBlock->Move(DOWN); // move the focus block
			force_down_counter = 0;   // reset our counter
		}
	}

	// Check to see if focus block's bottom has hit something. If it has, we decrement our counter. //
	if ( CheckWallCollisions(g_FocusBlock, DOWN) || CheckEntityCollisions(g_FocusBlock, DOWN) )
	{
		slide_counter--;
	}
	// If there isn't a collision, we reset our counter.    //
	// This is in case the player moves out of a collision. //
	else 
	{
		slide_counter = SLIDE_TIME;
	}
	// If the counter hits zero, we reset it and call our //
	// function that handles changing the focus block.    //
	if (slide_counter == 0)
	{
		slide_counter = SLIDE_TIME;
		HandleBottomCollision();
	}
	drawBackground(GFX_TOP, background_bgr, background_bgr_size);
	g_FocusBlock->Draw();
	g_NextBlock->Draw();
	int i=0;
	while (i < g_OldSquares.size())
	{
		g_OldSquares[i]->Draw();
		i++;
	}
	gfxFlushBuffers();
    gfxSwapBuffers();
	srv_Flip();
	gspWaitForVBlank();
	printf("\x1b[0;0H");
	printf("FPS %f\n", CurrentFPS);
	printf("Level : %d\n", g_Level);
	printf("Score : %d\n", g_Score);
	printf("Needed Score : %d", g_Level*POINTS_PER_LEVEL);
	g_Timer = svcGetSystemTick();
}

void GameWon()
{
	ClearScreenBuffer(GFX_TOP);
	consoleClear();
	printf("\x1b[0;0H");
	printf("You Won!!!\nQuit Game (Yes(A) or No(B))?\n                               ");
	HandleWinLoseInput();
}

void GameLost()
{
	ClearScreenBuffer(GFX_TOP);
	consoleClear();
	printf("\x1b[0;0H");
	printf("You Lost!!!\nQuit Game (Yes(A) or No(B))?\n                               ");
	HandleWinLoseInput();
}

void Shutdown()
{
	cSquare** temp_array_1 = g_FocusBlock->GetSquares();
	cSquare** temp_array_2 = g_NextBlock->GetSquares();

	// Delete our blocks 
	delete g_FocusBlock;
	delete g_NextBlock;

	// Delete the temporary arrays of squares 
	for (int i=0; i<4; i++)
	{
		delete temp_array_1[i];
		delete temp_array_2[i];
	}
	
	// Delete the squares that are in the game area 
	for (int i=0; i<g_OldSquares.size(); i++)
	{
		delete g_OldSquares[i];
	}
}

bool down_pressed = false;

void HandleGameInput()
{
	hidScanInput();

	u32 kDown = hidKeysDown();
	u32 kUp = hidKeysUp();
	if(kDown & KEY_UP)
	{
		if (!CheckRotationCollisions(g_FocusBlock))
		{
			g_FocusBlock->Rotate();
		}
	}
	if(kDown & KEY_LEFT)
	{
		if ( !CheckWallCollisions(g_FocusBlock, LEFT) &&
			 !CheckEntityCollisions(g_FocusBlock, LEFT) )
		{
			g_FocusBlock->Move(LEFT);
		}
	}
	if(kDown & KEY_RIGHT)
	{
		if ( !CheckWallCollisions(g_FocusBlock, RIGHT) &&
			 !CheckEntityCollisions(g_FocusBlock, RIGHT) )
		{
			g_FocusBlock->Move(RIGHT);
		}
	}
	if(kDown & KEY_DOWN)
	{
		down_pressed = true;
	}
	
	if(kUp & KEY_DOWN)
	{
		down_pressed = false;
	}
	
	if(down_pressed)
	{
		if ( !CheckWallCollisions(g_FocusBlock, DOWN) &&
			 !CheckEntityCollisions(g_FocusBlock, DOWN) )
		{
			g_FocusBlock->Move(DOWN);
		}
	}

	if (kDown & KEY_START)
		quit = true;
}

void HandleWinLoseInput()
{
	u32 kDown = 0;
	while(!(kDown & KEY_A) && !(kDown & KEY_B))
	{
		hidScanInput();
		kDown = hidKeysDown();
		if (kDown & KEY_A)
		{
			for (int i=0; i<g_OldSquares.size(); i++)
			{
				delete g_OldSquares[i];
			}
			g_OldSquares.clear();
			Init();
			return;  
		}
		// If player chooses to continue playing, we pop off    //
		// current state and push exit and menu states back on. //
		if (kDown & KEY_B)
		{
			quit = true;
			return;  
		}
	}
}

bool CheckEntityCollisions(cSquare* square, Direction dir) 
{ 
    // Width/height of a square. Also the distance 
    // between two squares if they've collided. 
    int distance = SQUARE_MEDIAN * 2;

    // Center of the given square 
    int centerX = square->GetCenterX(); 
    int centerY = square->GetCenterY();

    // Determine the location of the square after moving
    switch (dir)
    {
        case DOWN:
        { 
            centerY += distance;
        } break;

        case LEFT:
        {
            centerX -= distance;
        } break;

        case RIGHT:
        {
            centerX += distance;
        } break;
    }

    // Iterate through the old squares vector, checking for collisions
    for (int i=0; i<g_OldSquares.size(); i++)
    {
        if ( ( abs(centerX - g_OldSquares[i]->GetCenterX() ) < distance ) &&
            ( abs(centerY - g_OldSquares[i]->GetCenterY() ) < distance ) )
        {
            return true;
        }
    }

    return false;
}

bool CheckEntityCollisions(cBlock* block, Direction dir) 
{ 
    // Get an array of the squares that make up the given block 
    cSquare** temp_array = block->GetSquares();

    // Now just call the other CheckEntityCollisions() on each square 
    for (int i=0; i<4; i++)
    {
        if ( CheckEntityCollisions(temp_array[i], dir) )
            return true;
    }

    return false;
}

bool CheckWallCollisions(cSquare* square, Direction dir) 
{
    // Get the center of the square
    int x = square->GetCenterX();
    int y = square->GetCenterY();

    // Get the location of the square after moving and see if its out of bounds 
    switch (dir)
    {
        case DOWN:
        {
            if ( (y + (SQUARE_MEDIAN*2)) > GAME_AREA_BOTTOM )
            {
                return true;
            }
            else
            {
                return false;
            }
        } break;

        case LEFT:
        {
            if ( (x - (SQUARE_MEDIAN*2)) < GAME_AREA_LEFT )
            {
                return true;
            }
            else
            {
                return false;
            }
        } break;

        case RIGHT:
        {
            if ( (x + (SQUARE_MEDIAN*2)) > GAME_AREA_RIGHT )
            {
                return true;
            }
            else
            {
                return false;
            }
        } break;
    }

    return false;
}

bool CheckWallCollisions(cBlock* block, Direction dir) 
{
    // Get an array of squares that make up the given block
    cSquare** temp_array = block->GetSquares();

    // Call other CheckWallCollisions() on each square 
    for (int i=0; i<4; i++)
    {
        if ( CheckWallCollisions(temp_array[i], dir) )
        return true;
    }

    return false;
}

bool CheckRotationCollisions(cBlock* block) 
{
    // Get an array of values for the locations of the rotated block's squares
    int* temp_array = block->GetRotatedSquares();

    // Distance between two touching squares 
    int distance = SQUARE_MEDIAN * 2;

    for (int i=0; i<4; i++)
    {
        // Check to see if the block will go out of bounds 
        if ( (temp_array[i*2] < GAME_AREA_LEFT) ||
             (temp_array[i*2] > GAME_AREA_RIGHT) )
        {
            delete temp_array;
            return true;
        }

        if ( temp_array[i*2+1] > GAME_AREA_BOTTOM )
        {
            delete temp_array;
            return true;
        }

        // Check to see if the block will collide with any squares 
        for (int index=0; index<g_OldSquares.size(); index++)
        {
            if ( ( abs(temp_array[i*2] - g_OldSquares[index]->GetCenterX()) < distance ) && 
                ( abs(temp_array[i*2+1] - g_OldSquares[index]->GetCenterY()) < distance ) )
            {
                delete temp_array;
                return true;
            }
        }
    }

    delete temp_array;
    return false;
}

void HandleBottomCollision()
{ 
    ChangeFocusBlock();

    // Check for completed lines and store the number of lines completed 
    int num_lines = CheckCompletedLines();

    if ( num_lines > 0 )
    {
        // Increase player's score according to number of lines completed 
        g_Score += POINTS_PER_LINE * num_lines;

        // Check to see if it's time for a new level 
        if (g_Score >= g_Level * POINTS_PER_LEVEL)
        {
            g_Level++;
			ClearScreenBuffer(GFX_BOTTOM);
            CheckWin(); // check for a win after increasing the level 
            g_FocusBlockSpeed -= SPEED_CHANGE; // shorten the focus blocks movement interval
        }
    }

    // Now would be a good time to check to see if the player has lost 
    CheckLoss();
}

void ChangeFocusBlock()
{
    // Get an array of pointers to the focus block squares
    cSquare** square_array = g_FocusBlock->GetSquares();

    // Add focus block squares to g_OldSquares 
    for (int i=0; i<4; i++)
    {
        g_OldSquares.push_back(square_array[i]);
    }

    delete g_FocusBlock; // delete the current focus block
    g_FocusBlock = g_NextBlock; // set the focus block to the next block
    g_FocusBlock->SetupSquares(BLOCK_START_X, BLOCK_START_Y);

    // Set the next block to a new block of random type 
    g_NextBlock = new cBlock(NEXT_BLOCK_CIRCLE_X, NEXT_BLOCK_CIRCLE_Y, (BlockType)(rand()%7));
}

int CheckCompletedLines()
{
    // Store the amount of squares in each row in an array
    int squares_per_row[13];

    // The compiler will fill the array with junk values if we don't do this 
    for (int index=0; index<13; index++)
        squares_per_row[index] = 0;

    int row_size = SQUARE_MEDIAN * 2; // pixel size of one row
    int bottom = GAME_AREA_BOTTOM - SQUARE_MEDIAN; // center of bottom row
    int top = bottom - (12 * row_size); // center of top row

    int num_lines = 0; // number of lines cleared 
    int row; // multipurpose variable

    // Check for full lines 
    for (int i=0; i<g_OldSquares.size(); i++)
    {
        // Get the row the current square is in 
        row = (g_OldSquares[i]->GetCenterY() - top) / row_size;

        // Increment the appropriate row counter
        squares_per_row[row]++; 
    }

    // Erase any full lines
    for (int line=0; line<13; line++)
    {
        // Check for completed lines 
        if (squares_per_row[line] == SQUARES_PER_ROW)
        {
            // Keep track of how many lines have been completed 
            num_lines++;

            // Find any squares in current row and remove them 
            for (int index=0; index<g_OldSquares.size(); index++)
            {
                if ( ( (g_OldSquares[index]->GetCenterY() - top) / row_size ) == line )
                {
                    // delete the square
                    delete g_OldSquares[index]; 
                    // remove it from the vector
                    g_OldSquares.erase(g_OldSquares.begin() + index);

                    // When we delete a square, the next square in the vector takes 
                    // its place. We have to be sure to stay at the current index so 
                    // we don't skip any squares. For example, if we delete the first 
                    // square, the second square now becomes the first. We have to 
                    // stay at the current (first) index so we can check the second 
                    // square (now the first). 
                    index--; 
                }
            }
        }
    }

    // Move squares above cleared line down
    for (int index=0; index<g_OldSquares.size(); index++)
    {
        for (int line=0; line<13; line++)
        {
            // Determine if this row was filled 
            if (squares_per_row[line] == SQUARES_PER_ROW)
            {
                // If it was, get the location of it within the game area
                row = (g_OldSquares[index]->GetCenterY() - top) / row_size;

                // Now move any squares above that row down one 
                if ( row < line )
                {
                    g_OldSquares[index]->Move(DOWN);
                }
            }
        }
    }

    return num_lines;
}

void CheckWin() 
{
    // If current level is greater than number of levels, player has won 
    if (g_Level > NUM_LEVELS)
    {
        GameWon();
    }
}

void CheckLoss() 
{
    // We call this function when the focus block is at the top of that 
    // game area. If the focus block is stuck now, the game is over. 
    if ( CheckEntityCollisions(g_FocusBlock, DOWN) )
    {
        // Clear the old squares vector 
        for (int i=0; i<g_OldSquares.size(); i++)
        {
            delete g_OldSquares[i];
        }
        g_OldSquares.clear();

        GameLost();
    }
}