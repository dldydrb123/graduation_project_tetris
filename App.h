#pragma once

#include "resource.h"
#include <string>


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif



#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


class MainApp
{
public:
    MainApp();
    ~MainApp();

    HRESULT Initialize();

    std::wstring player1Name, player2Name;

    // Process and dispatch messages
    void RunMessageLoop();
    void GameLoop();

private:

    HWND m_hwnd;

    Engine* engine;

    // The windows procedure.
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};