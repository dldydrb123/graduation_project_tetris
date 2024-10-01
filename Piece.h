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
	bool StackCollision(Matrix* stackCells);
	Point2D GetPosition();
	Matrix* GetCells();

	// ������ �׸��� ���� ����
	D2D1_COLOR_F colors[8] = {
		D2D1::ColorF(D2D1::ColorF::Black),      // 0
		D2D1::ColorF(D2D1::ColorF::Blue),       // 1
		D2D1::ColorF(D2D1::ColorF::Green),      // 2
		D2D1::ColorF(D2D1::ColorF::Yellow),     // 3
		D2D1::ColorF(D2D1::ColorF::Cyan),       // 4
		D2D1::ColorF(D2D1::ColorF::Magenta),    // 5
		D2D1::ColorF(D2D1::ColorF::Orange),     // 6
		D2D1::ColorF(D2D1::ColorF::Red)         // 7
	};
private:
	Point2D position;
	Matrix* cells;

	bool waiting;

	bool cellsTemplates[7][4][4] = { // �������� ������ֽ��ϴ�.
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

	ID2D1SolidColorBrush* m_pBrushes[8];
	ID2D1SolidColorBrush* m_pSelectedBrush;
};
