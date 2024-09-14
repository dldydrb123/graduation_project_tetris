#pragma once

#include "resource.h"
#include "Piece.h"
#include "Stack.h"

#define RESOLUTION_X 1000
#define RESOLUTION_Y 1000

#define CELL_SIZE 20

class Engine
{
public:
	Engine();
	~Engine();

	HRESULT InitializeD2D(HWND m_hwnd);
	void KeyUp(WPARAM wParam);
	void KeyDown(WPARAM wParam);
	void MousePosition(int x, int y);
	void MouseButtonUp(bool left, bool right);
	void MouseButtonDown(bool left, bool right);
	void Logic(double elapsedTime);
	HRESULT Draw();

private:
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;

	IDWriteFactory* m_pDWriteFactory;
	IDWriteTextFormat* m_pTextFormat;
	ID2D1SolidColorBrush* m_pWhiteBrush;

	void InitializeTextAndScore();
	void DrawTextAndScore();

	Stack* stack;
	Piece* activePiece;
	Piece* waitingPiece;
	Stack* stack2;
	Piece* activePiece2;
	Piece* waitingPiece2;

	bool gameOver = false;
	bool gameOver2 = false;

	bool downPressed = false;
	bool downPressed2 = false;
	bool leftPressed = false;
	bool leftPressed2 = false;
	bool rightPressed = false;
	bool rightPressed2 = false;
	bool spacePressed = false;
	bool spacePressed2 = false;
	bool over = false;

	int score = 0;

	double autoFallDelay;
	double autoFallAccumulated;
	double keyPressDelay;
	double keyPressAccumulated;
	double autoFallDelay2;
	double autoFallAccumulated2;
	double keyPressDelay2;
	double keyPressAccumulated2;
};

