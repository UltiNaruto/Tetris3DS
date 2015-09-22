//////////////////////////////////////////////////////////////////////////////////
// cBlock.h
// - represents a block in Falling Blocks
// - this class is just four cSquare's placed in a specific shape
//////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cSquare.h"

class cBlock
{
private:
	// Location of the center of the block //
	int m_CenterX;
	int m_CenterY;

	// Type of block //
	BlockType m_Type;

	// Array of squares that make up the block //
	cSquare* m_Squares[4];

public:
	// The constructor just sets the block location and calls SetupSquares //
	cBlock(int x, int y, BlockType type)  : m_CenterX(x), m_CenterY(y),
		m_Type(type)
	{
		for (int i=0; i<4; i++)
		{
			m_Squares[i] = NULL;
		}

		SetupSquares(x, y);		
	}

	// Setup our block according to its location and type. Note that the squares //
	// are defined according to their distance from the block's center. This     //
	// function takes a surface that gets passed to cSquare's constructor.       //
	void SetupSquares(int x, int y)
	{
		// This function takes the center location of the block. We set our data //
		// members to these values to make sure our squares don't get defined    //
		// around a new center without our block's center values changing too.   //
		m_CenterX = x;
		m_CenterY = y;

		// Make sure that any current squares are deleted //
		for (int i=0; i<4; i++)
		{
			if (m_Squares[i])
				delete m_Squares[i];
		}

		switch (m_Type)
		{
		case SQUARE_BLOCK:
			{
				// Upper left //
				m_Squares[0] = new cSquare(x - SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// Lower Left //
				m_Squares[1] = new cSquare(x - SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				// Upper right //
				m_Squares[2] = new cSquare(x + SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// Lower right //
				m_Squares[3] = new cSquare(x + SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);			
			} break;
		case T_BLOCK:
			{
				// Top //
				m_Squares[0] = new cSquare(x + SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// Middle //
				m_Squares[1] = new cSquare(x + SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				// Left //
				m_Squares[2] = new cSquare(x - SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				// Right //
				m_Squares[3] = new cSquare(x + (SQUARE_MEDIAN * 3), y + SQUARE_MEDIAN, m_Type);				
			} break;
		case L_BLOCK:
			{
				// |  //
				m_Squares[0] = new cSquare(x - SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// |  //
				m_Squares[1] = new cSquare(x - SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				// |_ //
				m_Squares[2] = new cSquare(x - SQUARE_MEDIAN, y + (SQUARE_MEDIAN * 3), m_Type);
				      // __ //
				m_Squares[3] = new cSquare(x + SQUARE_MEDIAN, y + (SQUARE_MEDIAN * 3), m_Type);				
			} break;
		case BACKWARDS_L_BLOCK:
			{
				    //  | //
				m_Squares[0] = new cSquare(x + SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				    //  | //
				m_Squares[1] = new cSquare(x + SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				    // _| //
				m_Squares[2] = new cSquare(x + SQUARE_MEDIAN, y + (SQUARE_MEDIAN * 3), m_Type);
				// __ //
				m_Squares[3] = new cSquare(x - SQUARE_MEDIAN, y + (SQUARE_MEDIAN * 3), m_Type);			
			} break;
		case STRAIGHT_BLOCK:
			{
				// Top //
				m_Squares[0] = new cSquare(x + SQUARE_MEDIAN, y - (SQUARE_MEDIAN * 3), m_Type);
				m_Squares[1] = new cSquare(x + SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				m_Squares[2] = new cSquare(x + SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				m_Squares[3] = new cSquare(x + SQUARE_MEDIAN, y + (SQUARE_MEDIAN * 3), m_Type);
				// Bottom //
			} break;
		case S_BLOCK:
			{
				// Top right //       
				m_Squares[0] = new cSquare(x + (SQUARE_MEDIAN * 3), y - SQUARE_MEDIAN, m_Type);
				// Top middle //
				m_Squares[1] = new cSquare(x + SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// Bottom middle //
				m_Squares[2] = new cSquare(x + SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				// Bottom left //
				m_Squares[3] = new cSquare(x - SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
			} break;
		case BACKWARDS_S_BLOCK:
			{
				// Top left //       
				m_Squares[0] = new cSquare(x - SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// Top middle //
				m_Squares[1] = new cSquare(x + SQUARE_MEDIAN, y - SQUARE_MEDIAN, m_Type);
				// Bottom middle //
				m_Squares[2] = new cSquare(x + SQUARE_MEDIAN, y + SQUARE_MEDIAN, m_Type);
				// Bottom right //
				m_Squares[3] = new cSquare(x + (SQUARE_MEDIAN * 3), y + SQUARE_MEDIAN, m_Type);
			} break;
		}
	}

	// Draw() simply iterates through the squares and calls their Draw() functions. //
	void Draw()
	{
		for (int i=0; i<4; i++)
		{
			m_Squares[i]->Draw();
		}
	}

	// Move() simply changes the block's center and calls the squares' move functions. //
	void Move(Direction dir)
	{
		switch (dir)
		{
		case LEFT:
			{
				m_CenterX -= SQUARE_MEDIAN * 2;
			} break;
		case RIGHT:
			{
				m_CenterX += SQUARE_MEDIAN * 2;
			} break;
		case DOWN:
			{
				m_CenterY += SQUARE_MEDIAN*2;
			} break;
		}

		for (int i=0; i<4; i++)
		{
			m_Squares[i]->Move(dir);
		}
	}

	// This function is explained in the tutorial. //
	void Rotate()
	{
		int x1, y1, x2, y2;

		for (int i=0; i<4; i++)
		{
			x1 = m_Squares[i]->GetCenterX(); 
			y1 = m_Squares[i]->GetCenterY();

			x1 -= m_CenterX;
			y1 -= m_CenterY;

			x2 = - y1;
			y2 = x1;

			x2 += m_CenterX;
			y2 += m_CenterY;

			m_Squares[i]->SetCenterX(x2);
			m_Squares[i]->SetCenterY(y2);
		}
	}

	// This function gets the locations of the squares after //
	// a rotation and returns an array of those values.      //
	int* GetRotatedSquares()
	{
		int* temp_array = new int[8];
		int x1, y1, x2, y2;

		for (int i=0; i<4; i++)
		{
			x1 = m_Squares[i]->GetCenterX(); 
			y1 = m_Squares[i]->GetCenterY();

			x1 -= m_CenterX;
			y1 -= m_CenterY;

			x2 = - y1;
			y2 = x1;

			x2 += m_CenterX;
			y2 += m_CenterY;

			temp_array[i*2]   = x2;
			temp_array[i*2+1] = y2;
		}

		return temp_array;
	}

	// This returns an array of pointers to the squares of the block. //
	cSquare** GetSquares()
	{
		return m_Squares;
	}
};

//  Aaron Cox, 2004 //