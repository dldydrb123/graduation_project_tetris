#pragma once

#include "resource.h"
#include "Matrix.h"

#define STACK_WIDTH 10
#define STACK_HEIGHT 20

class Stack
{
public:
	Stack();
	~Stack();

	void InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget);
	int RemoveLines();
	void Draw(ID2D1HwndRenderTarget* m_pRenderTarget);
	void Draw2(ID2D1HwndRenderTarget* m_pRenderTarget);

	Matrix* GetCells();
	Matrix* GetCells2();
private:
	Matrix* cells;
	Matrix* cells2;

	ID2D1SolidColorBrush* m_pBlueBrush;
	ID2D1SolidColorBrush* m_pGreenBrush;
	ID2D1SolidColorBrush* m_pGreenBrush2;
	ID2D1SolidColorBrush* m_pYellowBrush;

};

