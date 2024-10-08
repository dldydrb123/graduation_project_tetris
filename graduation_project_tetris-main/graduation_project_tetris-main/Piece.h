#pragma once

#include "resource.h"
#include "Point2D.h"
#include "Matrix.h"

class Piece
{
public:
	Piece();
	~Piece();

	int BrushIndex;
	int GetRandomBrushIndex();
	void InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget);
	void Activate();
	bool Advance(Matrix* stackCells);
	void GoLeft(Matrix* stackCells);
	void GoRight(Matrix* stackCells);
	void Rotate(Matrix* stackCells);

	void Draw(ID2D1HwndRenderTarget* m_pRenderTarget);
	void Draw2(ID2D1HwndRenderTarget* m_pRenderTarget);

	bool LeftWallCollision();
	bool RightWallCollision();
	int StackCollision(Matrix* stackCells);
	Point2D GetPosition();
	Matrix* GetCells();

	// 블럭을 그리는 색깔 정의
	D2D1_COLOR_F colors[9] = {
		D2D1::ColorF(D2D1::ColorF::Black),      // 0
		D2D1::ColorF(D2D1::ColorF::Blue),       // 1
		D2D1::ColorF(D2D1::ColorF::Green),      // 2
		D2D1::ColorF(D2D1::ColorF::Yellow),     // 3
		D2D1::ColorF(D2D1::ColorF::Cyan),       // 4
		D2D1::ColorF(D2D1::ColorF::Magenta),    // 5
		D2D1::ColorF(D2D1::ColorF::Orange),     // 6
		D2D1::ColorF(D2D1::ColorF::Red),        // 7
		D2D1::ColorF(D2D1::ColorF::White)       // 8
	};
private:
	Point2D position;
	Matrix* cells;

	bool waiting;

	int cellsTemplates[7][4][4] = { // 블럭들이 저장되어있습니다.
						{ { 0, 0, 0, 0 },
						  { 0, 0, 0, 0 },
						  { 1, 1, 1, 1 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 1, 1 },
						  { 0, 1, 0, 0 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 0, 0 },
						  { 0, 1, 1, 1 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 1, 1 },
						  { 0, 0, 1, 0 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 1, 0 },
						  { 0, 1, 1, 0 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 1, 0 },
						  { 0, 0, 1, 1 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 0, 1, 1 },
						  { 0, 1, 1, 0 },
						  { 0, 0, 0, 0 } }
	};

	int Itemcells[7][4][4] = { // 아이템이 포함된 블럭
						{ { 0, 0, 0, 0 },
						  { 0, 0, 0, 0 },
						  { 1, 2, 1, 1 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 2, 1 },
						  { 0, 1, 0, 0 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 2, 0, 0 },
						  { 0, 1, 1, 1 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 2, 1 },
						  { 0, 0, 1, 0 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 1, 0 },
						  { 0, 2, 1, 0 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 1, 2, 0 },
						  { 0, 0, 1, 1 },
						  { 0, 0, 0, 0 } },

						{ { 0, 0, 0, 0 },
						  { 0, 0, 1, 1 },
						  { 0, 2, 1, 0 },
						  { 0, 0, 0, 0 } }
	};

	ID2D1SolidColorBrush* m_pBrushes[9];
	ID2D1SolidColorBrush* m_pSelectedBrush;
};

