// Tetris.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Engine.h"
#include "App.h"
#include <thread>
#include <atomic>
#include <socketapi.h>



// 게임 루프 실행 상태를 관리할 변수
std::atomic<bool> running(true);
HWND hEdit1P, hEdit2P;

#pragma comment(lib, "d2d1")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

     if (SUCCEEDED(CoInitialize(NULL)))
    {
        MainApp app;

        if (SUCCEEDED(app.Initialize()))
        {
            app.RunMessageLoop();
        }

        CoUninitialize();
    }
    return 0;
}

MainApp::MainApp() : m_hwnd(NULL)
{
        engine = new Engine();

}

MainApp::~MainApp()
{
    // 동적으로 할당한 객체 삭제
    delete engine;
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
        
        if (GameStart)
        {
            // 게임 로직 업데이트 및 그리기
            engine->Logic(elapsed_secs);
            engine->Draw();
        }
        else
        {
            engine->Draw2();
        }
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

        // 버튼 생성
        HWND hButton = CreateWindow(
            L"BUTTON", L"Start Game",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            100, 100, 100, 50, // 버튼 위치 및 크기
            m_hwnd, (HMENU)ID_START_BUTTON, HINSTANCE(GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE)), NULL
        );
        if (!hButton) {
            MessageBox(m_hwnd, L"Start Game 버튼 생성에 실패했습니다.", L"오류", MB_OK);
            return HRESULT_FROM_WIN32(GetLastError());
        }

        // 1P 이름 입력 창
        hEdit1P = CreateWindow(
            L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            100, 50, 150, 20, // 위치와 크기
            m_hwnd, NULL, HINSTANCE(GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE)), NULL
        );

        // 2P 이름 입력 창
        hEdit2P = CreateWindow(
            L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            100, 80, 150, 20, // 위치와 크기
            m_hwnd, NULL, HINSTANCE(GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE)), NULL
        );
    hr = m_hwnd ? S_OK : E_FAIL;

    // m_hwnd가 유효한지 확인
    if (m_hwnd)
    {
        // GetWindowRect와 GetClientRect 호출
        RECT rect1;
        GetWindowRect(m_hwnd, &rect1);
        RECT rect2;
        GetClientRect(m_hwnd, &rect2);

        // 창 크기 조정
        SetWindowPos(
            m_hwnd,
            NULL,
            rect1.left,
            rect1.top,
            RESOLUTION_X + ((rect1.right - rect1.left) - (rect2.right - rect2.left)),
            RESOLUTION_Y + ((rect1.bottom - rect1.top) - (rect2.bottom - rect2.top)),
            NULL
        );
            // Direct2D 초기화
            engine->InitializeD2D(m_hwnd);


        // 윈도우 표시
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        UpdateWindow(m_hwnd);

        // 게임 루프 실행
        std::thread gameThread(&MainApp::GameLoop, this);
        gameThread.detach(); // 메인 스레드와 분리
    }
    else
    {
        hr = E_FAIL; // 윈도우 생성 실패 시 E_FAIL 반환
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

            case WM_COMMAND:
                // 버튼 클릭 이벤트 처리
                if (LOWORD(wParam) == ID_START_BUTTON) {
                    pMainApp -> GameStart = true; // GameStart 변수를 true로 설정
                    
                    // Start Game 버튼 클릭 시 동작
                    wchar_t name1[100];
                    GetWindowText(hEdit1P, name1, 100);
                    pMainApp -> player1Name = name1;

                    wchar_t name2[100];
                    GetWindowText(hEdit2P, name2, 100);
                    pMainApp-> player2Name = name2;

                    pMainApp->engine->setPlayerName(1, name1);
                    pMainApp->engine->setPlayerName(2, name2);

                    // 입력 창과 버튼을 숨기고 게임 시작
                    ShowWindow(hEdit1P, SW_HIDE);
                    ShowWindow(hEdit2P, SW_HIDE);
                    ShowWindow((HWND)lParam, SW_HIDE); // 버튼 숨기기
                    EnableWindow((HWND)lParam, FALSE); // 버튼 비활성화

                    // 버튼을 클릭한 후 메인 윈도우로 포커스 반환
                    SetFocus(hwnd);
                }
                break;
            case WM_KEYDOWN:
            {
                pMainApp->engine->KeyDown(wParam);
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

