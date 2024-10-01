#pragma once

#include "resource.h"
#include "Matrix.h"

#define STACK_WIDTH 10
#define STACK_HEIGHT 25

class Stack
{
public:
	Stack();
	~Stack();

	void InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget);
	int RemoveLines(Matrix* stackCells);
	int RemoveLines2(Matrix* stackCells);
	void Draw(ID2D1HwndRenderTarget* m_pRenderTarget);
	void Draw2(ID2D1HwndRenderTarget* m_pRenderTarget);

	Matrix* GetCells();
private:
	Matrix* cells;

	ID2D1SolidColorBrush* m_pBlueBrush;
	ID2D1SolidColorBrush* m_pSelectedBrush;
	ID2D1SolidColorBrush* m_pBrushes[8];
	ID2D1SolidColorBrush* m_pYellowBrush;
	// 블럭을 그리는 색깔 정의
	D2D1_COLOR_F colors[8] = {
		D2D1::ColorF(D2D1::ColorF::Black),        // 0
		D2D1::ColorF(D2D1::ColorF::Blue),       // 1
		D2D1::ColorF(D2D1::ColorF::Green),      // 2
		D2D1::ColorF(D2D1::ColorF::Yellow),     // 3
		D2D1::ColorF(D2D1::ColorF::Cyan),       // 4
		D2D1::ColorF(D2D1::ColorF::Magenta),    // 5
		D2D1::ColorF(D2D1::ColorF::Orange),     // 6
		D2D1::ColorF(D2D1::ColorF::Red)        // 7
	};
};

