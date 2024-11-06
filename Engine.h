#pragma once

#include "resource.h"
#include "Piece.h"
#include "Stack.h"
#include "Item.h"

#define RESOLUTION_X 900
#define RESOLUTION_Y 800

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
	HRESULT Draw2();

	void SaveScore(const std::string& fileName, int score1, const std::wstring& player1Name, int score2, const std::wstring& player2Name);
	int LoadScore(const std::string& fileName);

	void setPlayerName(int playerIndex, const std::wstring& name); // 함수 선언

private:
	std::wstring player1Name, player2Name; // player1Name 변수 선언

	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;

	IDWriteFactory* m_pDWriteFactory;
	IDWriteTextFormat* m_pTextFormat;
	ID2D1SolidColorBrush* m_pWhiteBrush;
	ID2D1SolidColorBrush* m_pItemBrush;

	void InitializeTextAndScore();
	HRESULT DrawTextAndScore();
	HRESULT DrawJpgImage(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pWICFactory, const wchar_t* filename, float x, float y, float width, float height);
	HRESULT DrawImage();

	Stack* stack;
	Piece* activePiece;
	Piece* waitingPiece;
	Piece* changePiece;

	Stack* stack2;
	Piece* activePiece2;
	Piece* waitingPiece2;
	Piece* changePiece2;

	int fcheck;
	int scheck;
	int height;
	int height2;
	int blindcheck;
	int blindcheck2;

	double fall;
	double fall2;
	double itemfall;
	double itemfall2;

	bool dualcheck = false;
	bool dualcheck2 = false;
	bool fallcheck = false;
	bool fallcheck2 = false;
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
	bool enteringPressed = false;
	bool enteringPressed2 = false;
	bool scoreSaved = false; // Declare this as a class member variable.

	int score = 0;
	int score2 = 0;

	double autoFallDelay;
	double autoFallAccumulated;
	double keyPressDelay;
	double keyPressAccumulated;
	double autoFallDelay2;
	double autoFallAccumulated2;
	double keyPressDelay2;
	double keyPressAccumulated2;
};

