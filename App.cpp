// Tetris.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Engine.h"
#include "App.h"
#include <thread>
#include <atomic>
#include <socketapi.h>
#include <gdiplus.h>  // GDI+ 헤더 포함

#pragma comment (lib,"Gdiplus.lib")  // GDI+ 라이브러리 링크

// 전역 변수
ULONG_PTR gdiplusToken;

// GDI+ 초기화 함수
void InitGDIPlus() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

// GDI+ 종료 함수
void ShutdownGDIPlus() {
    Gdiplus::GdiplusShutdown(gdiplusToken);
}



// 게임 루프 실행 상태를 관리할 변수
std::atomic<bool> running(true);

#pragma comment(lib, "d2d1")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // GDI+ 초기화
    InitGDIPlus();

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        MainApp app;

        if (SUCCEEDED(app.Initialize()))
        {
            app.RunMessageLoop();
        }

        CoUninitialize();
    }
    // GDI+ 종료
    ShutdownGDIPlus();

    return 0;
}

MainApp::MainApp() : m_hwnd(NULL)
{
    engine = new Engine();
}

MainApp::~MainApp()
{
}

void MainApp::GameLoop()
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    int frames = 0;
    double framesTime = 0;

    while (running)
    {
        end = std::chrono::steady_clock::now();
        double elapsed_secs = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
        begin = end;

        // FPS 표시
        framesTime += elapsed_secs;
        frames++;
        if (framesTime > 1)
        {
            WCHAR fpsText[32];
            swprintf(fpsText, 32, L"Game: %d FPS", frames);
            SetWindowText(m_hwnd, fpsText); // app 대신 m_hwnd 사용
            frames = 0;
            framesTime = 0;
        }

        // 게임 로직 업데이트 및 그리기
        engine->Logic(elapsed_secs);
        engine->Draw();
    }
}

void MainApp::RunMessageLoop()
{
    MSG msg;

    while (running)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                running = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

HRESULT MainApp::Initialize()
{
    HRESULT hr = S_OK;

    // 윈도우 창 만드는 부분
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = HINST_THISCOMPONENT;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    wcex.lpszClassName = L"D2DMainApp";

    ATOM x = RegisterClassEx(&wcex);

    m_hwnd = CreateWindowEx(
        NULL,
        L"D2DMainApp",
        L"Game",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        RESOLUTION_X,
        RESOLUTION_Y,
        NULL,
        NULL,
        HINST_THISCOMPONENT,
        this
    );
    hr = m_hwnd ? S_OK : E_FAIL;

    RECT rect1;
    GetWindowRect(m_hwnd, &rect1);
    RECT rect2;
    GetClientRect(m_hwnd, &rect2);

    SetWindowPos(
        m_hwnd,
        NULL,
        rect1.left,
        rect1.top,
        RESOLUTION_X + ((rect1.right - rect1.left) - (rect2.right - rect2.left)),
        RESOLUTION_Y + ((rect1.bottom - rect1.top) - (rect2.bottom - rect2.top)),
        NULL
    );

    if (SUCCEEDED(hr))
    {
        engine->InitializeD2D(m_hwnd);

        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);

        std::thread gameThread(&MainApp::GameLoop, this); // 멤버 함수 호출
        gameThread.detach(); // 메인 스레드와 분리
    }

    return hr;
}

LRESULT CALLBACK MainApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        MainApp* pMainApp = (MainApp*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pMainApp)
        );

        result = 1;
    }
    else
    {
        MainApp* pMainApp = reinterpret_cast<MainApp*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (pMainApp)
        {
            switch (message)
            {

            case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_KEYDOWN:
            {
                pMainApp->engine->KeyDown(wParam);
                //2P
                if (wParam == VK_LEFT)  // 왼쪽 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\left.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 538, 690, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == VK_RIGHT)  // 오른쪽 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\right.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 679, 690, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == VK_UP)  // 위쪽 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\up.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 608, 640, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == VK_DOWN)  // 아래 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\down.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 608, 690, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == VK_RETURN)  // 스페를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\space2.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 538, 750, 210, 30);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                //////////////////////////////////////////////////////////////////////////
                //1p
                if (wParam == 'A')  // 왼쪽 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\left.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 194, 690, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == 'D')  // 오른쪽 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\right.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 335, 690, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == 'W')  // 위쪽 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\up.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 264, 640, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == 'S')  // 아래 방향키를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\down.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 264, 690, 70, 50);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }

                if (wParam == VK_SPACE)  // 스페를 눌렀을 때
                {
                    HDC hdc = GetDC(hwnd);
                    // JPG 파일 로드 (GDI+ 사용)
                    Gdiplus::Graphics graphics(hdc);

                    // 하위 폴더 "image"에 있는 left.png 파일 경로
                    Gdiplus::Image image(L"image\\space2.png");

                    // 이미지 그리기 (50, 50 위치에 이미지 표시)
                    graphics.DrawImage(&image, 194, 750, 210, 30);

                    ReleaseDC(hwnd, hdc);  // DC 해제
                }
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_KEYUP:
            {
                pMainApp->engine->KeyUp(wParam);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_MOUSEMOVE:
            {
                pMainApp->engine->MousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_LBUTTONUP:
            {
                pMainApp->engine->MouseButtonUp(true, false);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_LBUTTONDOWN:
            {
                pMainApp->engine->MouseButtonDown(true, false);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_RBUTTONUP:
            {
                pMainApp->engine->MouseButtonUp(false, true);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_RBUTTONDOWN:
            {
                pMainApp->engine->MouseButtonDown(false, true);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            result = 1;
            wasHandled = true;
            break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
    return result;
}
