#pragma once

#include "resource.h"
#include "Point2D.h"
#include "Matrix.h"

class Piece
{
public:
	Piece();
	~Piece();

	void InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget);
	void Activate();
	bool Advance(Matrix* stackCells);
	bool Advance2(Matrix* stackCells2);
	void GoLeft(Matrix* stackCells);
	void GoRight(Matrix* stackCells);
	void Rotate(Matrix* stackCells);

	void Draw(ID2D1HwndRenderTarget* m_pRenderTarget);
	void Draw2(ID2D1HwndRenderTarget* m_pRenderTarget);

	bool LeftWallCollision();
	bool RightWallCollision();
	bool StackCollision(Matrix* stackCells);
	bool StackCollision2(Matrix* stackCells);
	Point2D GetPosition();
	Point2D GetPosition2();
	Matrix* GetCells();
	Matrix* GetCells2();

private:
	Point2D position;
	Point2D position2;
	Matrix* cells;
	Matrix* cells2;

	bool waiting; // Is the current piece or the next piece?

	bool cellsTemplates[7][4][4] = { // This is where we define our pieces templates
						{ { false, false, false, false },
						  { false, false, false, false },
						  { true,  true,  true,  true  },
						  { false, false, false, false } },

						{ { false, false, false, false },
						  { false, true,  true,  true  },
						  { false, true,  false, false },
						  { false, false, false, false } },

						{ { false, false, false, false },
						  { false, true,  false, false },
						  { false, true,  true,  true  },
						  { false, false, false, false } },

						{ { false, false, false, false },
						  { false, true,  true,  true  },
						  { false, false, true,  false },
						  { false, false, false, false } },

						{ { false, false, false, false },
						  { false, true,  true,  false },
						  { false, true,  true,  false },
						  { false, false, false, false } },

						{ { false, false, false, false },
						  { false, true,  true,  false },
						  { false, false, true,  true  },
						  { false, false, false, false } },

						{ { false, false, false, false },
						  { false, false, true,  true  },
						  { false, true,  true,  false },
						  { false, false, false, false } }
	};

	ID2D1SolidColorBrush* m_pRedBrush;

};

