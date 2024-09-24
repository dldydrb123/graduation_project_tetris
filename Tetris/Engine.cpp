#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Piece.h"
#include "Engine.h"
#include "Item.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Windowscodecs.lib")

Engine::Engine() : m_pDirect2dFactory(NULL), m_pRenderTarget(NULL)
{
	// Constructor
	// Initialize your game elements here

    srand(time(NULL));
	
    stack = new Stack();
    stack2 = new Stack();

    activePiece = new Piece();
    activePiece->Activate();
    waitingPiece = new Piece();

    activePiece2 = new Piece();
    activePiece2->Activate();
    waitingPiece2 = new Piece();

    //블록 떨어지는 속도
    autoFallDelay = 100;
    autoFallAccumulated = 0;
    keyPressDelay = 0.07;
    keyPressAccumulated = 0;

    autoFallDelay2 = 0.7;
    autoFallAccumulated2 = 0;
    keyPressDelay2 = 0.07;
    keyPressAccumulated2 = 0;

    fcheck = 0;
    scheck = 0;
}

Engine::~Engine()
{
    // Destructor

    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);

    // Safe-release your game elements here
    delete stack;
    delete waitingPiece;
    delete activePiece;
    delete stack2;
    delete waitingPiece2;
    delete activePiece2;
}

HRESULT Engine::InitializeD2D(HWND m_hwnd)
{
    // Initializes Direct2D, for drawing
    D2D1_SIZE_U size = D2D1::SizeU(RESOLUTION_X, RESOLUTION_Y);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &m_pRenderTarget
    );

    // Initialize the D2D part of your game elements here
    InitializeTextAndScore();
    stack->InitializeD2D(m_pRenderTarget);
    activePiece->InitializeD2D(m_pRenderTarget);
    waitingPiece->InitializeD2D(m_pRenderTarget);

    stack2->InitializeD2D(m_pRenderTarget);
    activePiece2->InitializeD2D(m_pRenderTarget);
    waitingPiece2->InitializeD2D(m_pRenderTarget);

    return S_OK;
}

void Engine::InitializeTextAndScore()
{
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(m_pDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );

    m_pDWriteFactory->CreateTextFormat(
        L"Verdana",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20,
        L"", //locale
        &m_pTextFormat
    );

    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &m_pWhiteBrush
    );
}

void Engine::KeyUp(WPARAM wParam)
{
    // If keyup, un-set the keys flags
    // Don't do any logic here, you want to control the actual logic in the Logic method below
    if (wParam == VK_DOWN)
        downPressed2 = false;

    if (wParam == VK_LEFT)
        leftPressed2 = false;

    if (wParam == VK_RIGHT)
        rightPressed2 = false;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = false;

    if (wParam == 83)
        downPressed = false;

    if (wParam == 65)
        leftPressed = false;

    if (wParam == 68)
        rightPressed = false;

    if (wParam == VK_SPACE || wParam == 87)
        spacePressed = false;
}

void Engine::KeyDown(WPARAM wParam)
{
	// If keyup, set the keys flags
	// Don't do any logic here, you want to control the actual logic in the Logic method below
    if (wParam == VK_DOWN)
        downPressed2 = true;

    if (wParam == VK_LEFT)
        leftPressed2 = true;

    if (wParam == VK_RIGHT)
        rightPressed2 = true;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = true;

    if (wParam == 83)
        downPressed = true;

    if (wParam == 65)
        leftPressed = true;

    if (wParam == 68)
        rightPressed = true;

    if (wParam == VK_SPACE || wParam == 87)
        spacePressed = true;
    
    if (wParam == VK_HOME)
        ItemGet = true;

    if (wParam == VK_END)
        ItemGet2 = true;

    if (wParam == VK_INSERT)
        SItemGet = true;

    if (wParam == VK_DELETE)
        SItemGet2 = true;

    if (wParam == 49 && ItemGet)
        ItemUse = 1;

    if (wParam == 50 && SItemGet)
        ItemUse = 2;

    if (wParam == VK_NUMPAD1 && ItemGet2)
        ItemUse2 = 1;

    if (wParam == VK_NUMPAD2 && SItemGet2)
        ItemUse2 = 2;
}

void Engine::MousePosition(int x, int y)
{
    // Campture mouse position when the mouse moves
    // Don't do any logic here, you want to control the actual logic in the Logic method below
}

void Engine::MouseButtonUp(bool left, bool right)
{
    // If mouse button is released, set the mouse flags
    // Don't do any logic here, you want to control the actual logic in the Logic method below
}

void Engine::MouseButtonDown(bool left, bool right)
{
        // If mouse button is pressed, set the mouse flags
        // Don't do any logic here, you want to control the actual logic in the Logic method below
}

void Engine::Logic(double elapsedTime)
{
    // This is the logic part of the engine. It receives the elapsed time from the app class, in seconds.
    // It uses this value for a smooth and consistent movement, regardless of the CPU or graphics speed

    if (gameOver || gameOver2) // 게임오버 체크
    {
        over = true;

        return;
    }

    // We will need the stack in several places below
    Matrix* stackCells = stack->GetCells();
    Matrix* stackCells2 = stack2->GetCells();

    // Due to a high FPS, we can't consider the keys at every frame because movement will be very fast
    // So we're using a delay, and if enough time has passed we take a key press into consideration

    
    keyPressAccumulated += elapsedTime;
    if (keyPressAccumulated > keyPressDelay)
    {
        keyPressAccumulated = 0;
        
        if (leftPressed || rightPressed || spacePressed)
        {
            // Remove any full rows
            int removed = stack->RemoveLines(stackCells);
            if (removed > 0)
            {
                score += pow(2, removed) * 100;
                autoFallDelay = autoFallDelay * 0.98;
            }
        }

        // Move left or right
        if (leftPressed)
            activePiece->GoLeft(stackCells);
        if (rightPressed)
            activePiece->GoRight(stackCells);

        // Rotate
        if (spacePressed)
        {
            activePiece->Rotate(stackCells);
            spacePressed = false;
        }

        // Move down
        // On this one we will just set autoFallAccumulated to be high, because we have the down movemenet logic below
        if (downPressed)
            autoFallAccumulated = autoFallDelay + 1;
    }
    
    keyPressAccumulated2 += elapsedTime;
    if (keyPressAccumulated2 > keyPressDelay2)
    {
        keyPressAccumulated2 = 0;
        
        if (leftPressed2 || rightPressed2 || spacePressed2)
        {
            // Remove any full rows
            int removed = stack->RemoveLines2(stackCells2);
            if (removed > 0)
            {
                score += pow(2, removed) * 100;
                autoFallDelay2 = autoFallDelay2 * 0.98;
            }
        }

        // Move left or right
        if (leftPressed2)
            activePiece2->GoLeft(stackCells2);
        if (rightPressed2)
            activePiece2->GoRight(stackCells2);

        // Rotate
        if (spacePressed2)
        {
            activePiece2->Rotate(stackCells2);
            spacePressed2 = false;
        }

        // Move down
        // On this one we will just set autoFallAccumulated to be high, because we have the down movemenet logic below
        if (downPressed2)
            autoFallAccumulated2 = autoFallDelay2 + 1;
    }


    // The piece falls automatically after a delay
    autoFallAccumulated += elapsedTime;
    if (autoFallAccumulated > autoFallDelay) //여기
    {
       autoFallAccumulated = 0;

        // Remove any full rows
        int removed = stack->RemoveLines(stackCells);
        if (removed > 0)
        {
            score += pow(2, removed) * 100;
            autoFallDelay = autoFallDelay * 0.98;

            if ((score - scorecheck) >= 2000) {
                scorecheck += 2000;
                if (random == 0) {
                    ItemGet = true;
                }
                else {
                    SItemGet = true;
                }
            }
        }

        if (ItemUse2 == 2) {
            SItemGet2 = false;
            autoFallDelay = autoFallDelay - 0.6;

            ItemUse2 = 3;
        }

        if (fcheck == 3 && ItemUse2 == 3) {
            ItemUse2 = 0;
            fcheck = 0;

            autoFallDelay2 = autoFallDelay2 + 0.6;
        }

        // Move down the active piece
        bool isConflict = activePiece->Advance(stackCells);
        // If we have a conflict with the stack, it means we were sitting on the stack or bottom wall already
        if (isConflict)
        {
            // We add the piece to stack
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (activePiece->GetCells()->Get(j, i) == true)
                    {
                        int realx = activePiece->GetPosition().x + j;
                        int realy = activePiece->GetPosition().y + i;
                        stackCells->Set(realx, realy, true);
                    }
                }
            }
            fcheck++;

            // Delete active piece, activate the waiting piece and generate new waiting piece
            delete activePiece;
            activePiece = waitingPiece;
            activePiece->Activate();
            waitingPiece = new Piece();
            waitingPiece->InitializeD2D(m_pRenderTarget);

            // If we have a collision right after we generate the new piece, 
            // it means the stack is too high, so game over
            if (activePiece->StackCollision(stackCells))
                gameOver = true;
        }
    }

    autoFallAccumulated2 += elapsedTime;
    if (autoFallAccumulated2 > autoFallDelay2) //여기
    {
        autoFallAccumulated2 = 0;

        // Remove any full rows
        int removed = stack->RemoveLines2(stackCells2);
        if (removed > 0)
        {
            score += pow(2, removed) * 100;
            autoFallDelay2 = autoFallDelay2 * 0.98;

            if ((score - scorecheck) >= 2000) {
                scorecheck += 2000;
                if (random == 0) {
                    ItemGet2 = true;
                }
                else {
                    SItemGet2 = true;
                }
            }
        }


        if (ItemUse == 2) {
            SItemGet = false;

            autoFallDelay2 = autoFallDelay2 - 0.6;
            
            ItemUse = 3;
        }

        if (scheck == 3 && ItemUse == 3) {
            ItemUse = 0;
            scheck = 0;

            autoFallDelay2 = autoFallDelay2 + 0.6;
        }

        // Move down the active piece
        bool isConflict = activePiece2->Advance(stackCells2);
        // If we have a conflict with the stack, it means we were sitting on the stack or bottom wall already
        if (isConflict)
        {
            // We add the piece to stack
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (activePiece2->GetCells()->Get(j, i) == true)
                    {
                        int realx = activePiece2->GetPosition().x + j;
                        int realy = activePiece2->GetPosition().y + i;
                        stackCells2->Set(realx, realy, true);
                    }
                }
            }
            scheck++;

            // Delete active piece, activate the waiting piece and generate new waiting piece
            delete activePiece2;
            activePiece2 = waitingPiece2;
            activePiece2->Activate();
            waitingPiece2 = new Piece();
            waitingPiece2->InitializeD2D(m_pRenderTarget);

            // If we have a collision right after we generate the new piece, 
            // it means the stack is too high, so game over
            if (activePiece2->StackCollision(stackCells2))
                gameOver2 = true;
        }
    }

}

HRESULT Engine::Draw()
{
    // This is the drawing method of the engine.
    // It runs every frame

    // Draws the elements in the game using Direct2D
    HRESULT hr;

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());


    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	
	// Below you can add drawing logic for your game elements
    
    stack->Draw(m_pRenderTarget);
    activePiece->Draw(m_pRenderTarget);
    waitingPiece->Draw(m_pRenderTarget);

    stack2->Draw2(m_pRenderTarget);
    activePiece2->Draw2(m_pRenderTarget);
    waitingPiece2->Draw2(m_pRenderTarget);
    DrawTextAndScore();

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}

void Engine::DrawTextAndScore()
{
    // Text and score
    int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
    int centerRight = RESOLUTION_X - (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;


    D2D1_RECT_F Next = D2D1::RectF(centerRight - 200, padding, centerRight + 200, padding + 100);
    m_pRenderTarget->DrawText(
        L"Next Piece",
        15,
        m_pTextFormat,
        Next,
        m_pWhiteBrush
    );


    D2D1_RECT_F ScoreLoca = D2D1::RectF(centerRight - 200, padding + 200, centerRight + 200, padding + 300);
    m_pRenderTarget->DrawText(
        L"Score",
        5,
        m_pTextFormat,
        ScoreLoca,
        m_pWhiteBrush
    );


    D2D1_RECT_F PScore = D2D1::RectF(centerRight - 200, padding + 300, centerRight + 200, padding + 400);
    WCHAR scoreStr[64];
    swprintf_s(scoreStr, L"%d        ", score);
    m_pRenderTarget->DrawText(
        scoreStr,
        7,
        m_pTextFormat,
        PScore,
        m_pWhiteBrush
    );

    D2D1_RECT_F TestView = D2D1::RectF(20, 20, 100, 100);
    if (ItemGet == true) {
        m_pRenderTarget->DrawText(
            L"한줄삭제 소지중",
            8,
            m_pTextFormat,
            TestView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F TestView2 = D2D1::RectF(720, 20, 800, 100);
    if (ItemGet2 == true) {
        m_pRenderTarget->DrawText(
            L"한줄삭제 소지중",
            8,
            m_pTextFormat,
            TestView2,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeView = D2D1::RectF(20, 120, 100, 150);
    if (SItemGet == true) {
        m_pRenderTarget->DrawText(
            L"속도증가 소지중",
            8,
            m_pTextFormat,
            SpeView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeView2 = D2D1::RectF(720, 120, 800, 150);
    if (SItemGet2 == true) {
        m_pRenderTarget->DrawText(
            L"속도증가 소지중",
            8,
            m_pTextFormat,
            SpeView2,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F UseView = D2D1::RectF(170, 550, 350, 650);
    if (ItemUse == 1) {
        m_pRenderTarget->DrawText(
            L"아이템 사용됨",
            7,
            m_pTextFormat,
            UseView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F UseView2 = D2D1::RectF(570, 550, 750, 650);
    if (ItemUse2 == 1) {
        m_pRenderTarget->DrawText(
            L"아이템 사용됨",
            7,
            m_pTextFormat,
            UseView2,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeedView = D2D1::RectF(130, 620, 350, 670);
    if (ItemUse2 == 2 || ItemUse2 == 3) {
        m_pRenderTarget->DrawText(
            L"속도증가 사용됨",
            8,
            m_pTextFormat,
            SpeedView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeedView2 = D2D1::RectF(550, 620, 750, 670);
    if (ItemUse == 2 || ItemUse == 3) {
        m_pRenderTarget->DrawText(
            L"속도증가 사용됨",
            8,
            m_pTextFormat,
            SpeedView2,
            m_pWhiteBrush
        );
    }

    if (over == true) {
        D2D1_RECT_F Over = D2D1::RectF(RESOLUTION_X / 2 - 200, RESOLUTION_Y / 2 + 200, RESOLUTION_X / 2 + 200, RESOLUTION_Y / 2 - 200);
        m_pRenderTarget->DrawText(
            L"Game Over!!",
            15,
            m_pTextFormat,
            Over,
            m_pWhiteBrush
        );
    }
}
