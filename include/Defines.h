#pragma once
#define WINDOW_WIDTH   400
#define WINDOW_HEIGHT  240

#define GAME_AREA_LEFT   111
#define GAME_AREA_RIGHT  311
#define GAME_AREA_BOTTOM 240

#define NUM_LEVELS       5    // number of levels in the game
#define POINTS_PER_LINE  525  // points player receives for completing a line
#define POINTS_PER_LEVEL 6300 // points player needs to advance a level

#define INITIAL_SPEED 60  // initial interval at which focus block moves down 
#define SPEED_CHANGE  10  // the above interval is reduced by this much each level

#define SLIDE_TIME      15
#define SQUARES_PER_ROW 10  // number of squares that fit in a row
#define SQUARE_MEDIAN   10  // distance from the center of a square to its sides

#define BLOCK_START_X 211
#define BLOCK_START_Y  0

#define LEVEL_RECT_X        42   // current level
#define LEVEL_RECT_Y        320
#define SCORE_RECT_X        42   // current score
#define SCORE_RECT_Y        340
#define NEEDED_SCORE_RECT_X 42   // score needed for next level
#define NEEDED_SCORE_RECT_Y 360
#define NEXT_BLOCK_CIRCLE_X 350  // next block in line to be focus block
#define NEXT_BLOCK_CIRCLE_Y 185

#define NS_TO_SECONDS(s) (s*1000ULL*1000ULL*1000ULL)

#ifndef REG_LCDBACKLIGHTSUB
#define REG_LCDBACKLIGHTSUB (u32)(0x1ED02A40 - 0x1EB00000)
#endif

#define FRAMES_PER_SECOND 30
#define FRAME_RATE        1000/FRAMES_PER_SECOND

#define STACKSIZE (4 * 1024)