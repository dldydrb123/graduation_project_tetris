#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Piece.h"
#include "Engine.h"
#include "Item.h"
#include <random>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Windowscodecs.lib")

Engine::Engine() : m_pDirect2dFactory(NULL), m_pRenderTarget(NULL)
{
    // 생성자입니다.
	// Engine.cpp 안에서 사용되는 변수들을 선언하는 공간입니다.

    srand(time(NULL));
	
    stack = new Stack();
    stack2 = new Stack();

    activePiece = new Piece();
    activePiece->Activate();
    waitingPiece = new Piece();

    activePiece2 = new Piece();
    activePiece2->Activate();
    waitingPiece2 = new Piece();

    // autoFall 자동으로 블럭이 떨어지는 속도.
    // 0.7 이 기본값입니다.
    autoFallDelay = 100;
    autoFallAccumulated = 0;
    keyPressDelay = 0.07;
    keyPressAccumulated = 0;

    autoFallDelay2 = 0.7;
    autoFallAccumulated2 = 0;
    keyPressDelay2 = 0.07;
    keyPressAccumulated2 = 0;

    // 아이템 사용을 확인하는 변수
    fcheck = 0;
    scheck = 0;
}

Engine::~Engine()
{
    // 닫힘

    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);

    // 만들었던 변수들 다시 삭제
    delete stack;
    delete waitingPiece;
    delete activePiece;
    delete stack2;
    delete waitingPiece2;
    delete activePiece2;
}

HRESULT Engine::InitializeD2D(HWND m_hwnd)
{
    // Direct 2D를 응용하는 부분입니다.
    D2D1_SIZE_U size = D2D1::SizeU(RESOLUTION_X, RESOLUTION_Y);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &m_pRenderTarget
    );

    // Dirext 2D에서 사용되는 변수들을 이용하는 부분입니다.
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
    // 글씨 폰트를 설정하는 부분입니다.
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
    // 키보드를 눌렀다가 땟을 때 입니다.
    // 입력이 끝나는 타이밍이니까 변수들이 false로 바꾸는걸 확인할수 있습니다.

    // 1p의 이동
    if (wParam == VK_DOWN)
        downPressed2 = false;

    if (wParam == VK_LEFT)
        leftPressed2 = false;

    if (wParam == VK_RIGHT)
        rightPressed2 = false;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = false;

    // 2p의 이동, 숫자들은 아스키코드로 W,A,S,D 입니다.
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
	// 키를 눌렀을때 입니다.
    // 눌렀을때니까 변수들이 true로 바뀌는걸 확인할수 있습니다.

    // 1p
    if (wParam == VK_DOWN)
        downPressed2 = true;

    if (wParam == VK_LEFT)
        leftPressed2 = true;

    if (wParam == VK_RIGHT)
        rightPressed2 = true;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = true;

    // 2p
    if (wParam == 83)
        downPressed = true;

    if (wParam == 65)
        leftPressed = true;

    if (wParam == 68)
        rightPressed = true;

    if (wParam == VK_SPACE || wParam == 87)
        spacePressed = true;
    
    // 테스트용 아이템 획득
    // 각각 방향키 위의 Home, End, Insert, Delete
    if (wParam == VK_HOME)
        ItemGet = true;

    if (wParam == VK_END)
        ItemGet2 = true;

    if (wParam == VK_INSERT)
        SItemGet = true;

    if (wParam == VK_DELETE)
        SItemGet2 = true;

    // 아이템 사용 확인
    // 1p는 키보드 상단 1, 2
    if (wParam == 49 && ItemGet)
        ItemUse = 1;

    if (wParam == 50 && SItemGet)
        ItemUse = 2;

    //2p는 키보드 우측 1, 2
    if (wParam == VK_NUMPAD1 && ItemGet2)
        ItemUse2 = 1;

    if (wParam == VK_NUMPAD2 && SItemGet2)
        ItemUse2 = 2;
}

// 마우스 관련 함수들인데 사용할지 말지 고민중
// 화면분활을 할거면 이거 넣고 메인화면이나 같은거 클릭되게 하는용
void Engine::MousePosition(int x, int y)
{
}

void Engine::MouseButtonUp(bool left, bool right)
{
}

void Engine::MouseButtonDown(bool left, bool right)
{
}

void Engine::Logic(double elapsedTime)
{
    // 로직
    // 본격적인 시스템 굴러가는 부분입니다.
    // 눈에 보이는 시스템들 대부분은 여기서 굴러간다고 생각해도 좋습니다.

    // 게임오버 체크
    // 둘 중 하나라도 게임오버시 over를 true로 바꾸고 리턴합니다.
    if (gameOver || gameOver2) 
    {
        over = true;

        return;
    }

    // Stack을 Metrix랑 연결해서 게임판을 만들어 냅니다. (테트리스 쌓이는 부분)
    Matrix* stackCells = stack->GetCells();
    Matrix* stackCells2 = stack2->GetCells();

    // 키보드를 눌러서 임의로 하강시키는 코드
    keyPressAccumulated += elapsedTime;
    if (keyPressAccumulated > keyPressDelay)
    {
        keyPressAccumulated = 0;
        
        // 각 이동 명령이 내려졌을때, 이동이 가능한지 확인합니다.
        if (leftPressed || rightPressed || spacePressed)
        {
            // 이동이 불가능 = 블럭이 땅에 떨어짐
            // 블럭이 땋에 닿았으니 RemoveLines를 호출해 꽉찬 열을 삭제합니다.
            int removed = stack->RemoveLines(stackCells);
            if (removed > 0)
            {
                //줄이 삭제되면 한 줄당 200점 씩 획득하고, 자동으로 블럭이 떨어지는 속도를 증가시킵니다.
                //autoFallDelay가 낮아질수록 빨리 떨어짐
                score += pow(2, removed) * 100;
                autoFallDelay = autoFallDelay * 0.98;
            }
        }

        // 왼쪽, 오른쪽 입력 확인후 각 함수 호출해 이동
        if (leftPressed)
            activePiece->GoLeft(stackCells);
        if (rightPressed)
            activePiece->GoRight(stackCells);

        // 블럭 돌리기
        if (spacePressed)
        {
            activePiece->Rotate(stackCells);
            spacePressed = false;
        }

        // 하강
        // 따로 변수가 있는건 아니고 그냥 AutoFall을 가속시켜서 빠르게 떨어트리는 방식입니다.
        if (downPressed)
            autoFallAccumulated = autoFallDelay + 1;
    }
    
    //2p용입니다, 내부는 1p용이랑 같습니다.
    keyPressAccumulated2 += elapsedTime;
    if (keyPressAccumulated2 > keyPressDelay2)
    {
        keyPressAccumulated2 = 0;

        // 각 이동 명령이 내려졌을때, 이동이 가능한지 확인합니다.
        if (leftPressed2 || rightPressed2 || spacePressed2)
        {
            // 이동이 불가능 = 블럭이 땅에 떨어짐
            // 블럭이 땋에 닿았으니 RemoveLines를 호출해 꽉찬 열을 삭제합니다.
            int removed = stack->RemoveLines2(stackCells2);
            if (removed > 0)
            {
                //줄이 삭제되면 한 줄당 200점 씩 획득하고, 자동으로 블럭이 떨어지는 속도를 증가시킵니다.
                //autoFallDelay가 낮아질수록 빨리 떨어짐
                score += pow(2, removed) * 100;
                autoFallDelay2 = autoFallDelay2 * 0.98;
            }
        }

        // 왼쪽, 오른쪽 입력 확인후 각 함수 호출해 이동
        if (leftPressed2)
            activePiece2->GoLeft(stackCells2);
        if (rightPressed2)
            activePiece2->GoRight(stackCells2);

        // 블럭 돌리기
        if (spacePressed2)
        {
            activePiece2->Rotate(stackCells2);
            spacePressed2 = false;
        }

        // 하강
        // 따로 변수가 있는건 아니고 그냥 AutoFall을 가속시켜서 빠르게 떨어트리는 방식입니다.
        if (downPressed2)
            autoFallAccumulated2 = autoFallDelay2 + 1;
    }


    // 블럭이 자동으로 떨어지는 부분입니다.
    // 블럭이 한칸 떨어질때마다 이 코드가 한번 실행된거라고 생각하면 됩니다.
    autoFallAccumulated += elapsedTime;
    if (autoFallAccumulated > autoFallDelay)
    {
       autoFallAccumulated = 0;

        // RemoveLines를 호출해 지울수 있는 열이 있는지 확인합니다.
        int removed = stack->RemoveLines(stackCells);
        if (removed > 0)
        {
            //위와 마찬가지로 지워지면 점수를 증가시키고 속도를 증가시킵니다.
            score += pow(2, removed) * 100;
            autoFallDelay = autoFallDelay * 0.98;

            //점수가 2000점에 도달할때마다 아이템을 획득합니다.
            if ((score - scorecheck) >= 2000) 
            {
                scorecheck += 2000;

                //랜덤한 값을 불러오기 위해 사용된 함수들입니다.
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(1, 100);

                int random = dis(gen) % 2;

                //랜덤값을 2로 나눈 나머지로 속도증가를 먹거나 한줄삭제를 먹습니다.
                if (random == 0) {
                    ItemGet = true;
                }
                else {
                    SItemGet = true;
                }
            }
        }

        // 상대의 속도증가 아이템 사용을 확인합니다.
        if (ItemUse2 == 2) {
            SItemGet2 = false;
            
            // 속도를 증가시키는 부분입니다.
            autoFallDelay = autoFallDelay - 0.6;

            // 아이템이 사용되었으니 사용중으로 변수를 바꿉니다.
            ItemUse2 = 3;
        }

        // 속도증가가 사용중인 부분입니다.
        // 1p가 블럭 3개를 떨어트리면 멈춥니다.
        if (fcheck == 3 && ItemUse2 == 3) {
            ItemUse2 = 0;
            fcheck = 0;

            autoFallDelay2 = autoFallDelay2 + 0.6;
        }

        // Advance 함수를 호출해 자동으로 블럭을 떨어트리는 부분입니다.
        bool isConflict = activePiece->Advance(stackCells);

        // 블럭이 떨어졌을때 그 떨어진 블럭을 스택에 쌓는 부분입니다.
        if (isConflict)
        {
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

            // 아이템이 사용되면 블럭이 떨어지는 걸 확인해 아이템의 사용시간을 체크합니다.
            if (ItemUse2 == 3)
                fcheck++;

            // 블럭을 스택에 쌓았으니 사용한 블럭을 제거하고
            // 다음 블럭을 가져와 사용 블럭으로 변경,
            // 다음 블럭을 새로운 블럭을 만들어 넣습니다.
            delete activePiece;
            activePiece = waitingPiece;
            activePiece->Activate();
            waitingPiece = new Piece();
            waitingPiece->InitializeD2D(m_pRenderTarget);

            // 활성 블럭이 스택과 바로 충돌이 일어났는지 확인합니다.
            // 활성 되자마자 스택과 충돌이 일어난거기 때문에 곧 게임 오버를 뜻합니다.
            if (activePiece->StackCollision(stackCells))
                gameOver = true;
        }
    }

    // 2p 버전입니다.
    autoFallAccumulated2 += elapsedTime;
    if (autoFallAccumulated2 > autoFallDelay2) 
    {
        autoFallAccumulated2 = 0;

        // RemoveLines를 호출해 지울수 있는 열이 있는지 확인합니다.
        int removed = stack->RemoveLines2(stackCells2);
        if (removed > 0)
        {
            //위와 마찬가지로 지워지면 점수를 증가시키고 속도를 증가시킵니다.
            score += pow(2, removed) * 100;
            autoFallDelay2 = autoFallDelay2 * 0.98;

            //점수가 2000점에 도달할때마다 아이템을 획득합니다.
            if ((score - scorecheck) >= 2000) {
                scorecheck += 2000;

                //랜덤한 값을 불러오기 위해 사용된 함수들입니다.
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(1, 100);

                int random = dis(gen) % 2;

                //랜덤값을 2로 나눈 나머지로 속도증가를 먹거나 한줄삭제를 먹습니다.
                if (random == 0) {
                    ItemGet2 = true;
                }
                else {
                    SItemGet2 = true;
                }
            }
        }

        // 상대의 속도증가 아이템 사용을 확인합니다.
        if (ItemUse == 2) {
            SItemGet = false;

            // 속도를 증가시키는 부분입니다.
            autoFallDelay2 = autoFallDelay2 - 0.6;

            // 아이템이 사용되었으니 사용중으로 변수를 바꿉니다.
            ItemUse = 3;
        }

        // 속도증가가 사용중인 부분입니다.
        // 1p가 블럭 3개를 떨어트리면 멈춥니다.
        if (scheck == 3 && ItemUse == 3) {
            ItemUse = 0;
            scheck = 0;

            autoFallDelay2 = autoFallDelay2 + 0.6;
        }

        // Advance 함수를 호출해 자동으로 블럭을 떨어트리는 부분입니다.
        bool isConflict = activePiece2->Advance(stackCells2);

        // 블럭이 떨어졌을때 그 떨어진 블럭을 스택에 쌓는 부분입니다.
        if (isConflict)
        {
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

            // 아이템이 사용되면 블럭이 떨어지는 걸 확인해 아이템의 사용시간을 체크합니다.
            if (ItemUse == 3)
                scheck++;

            // 블럭을 스택에 쌓았으니 사용한 블럭을 제거하고
            // 다음 블럭을 가져와 사용 블럭으로 변경,
            // 다음 블럭을 새로운 블럭을 만들어 넣습니다.
            delete activePiece2;
            activePiece2 = waitingPiece2;
            activePiece2->Activate();
            waitingPiece2 = new Piece();
            waitingPiece2->InitializeD2D(m_pRenderTarget);

            // 스택에 충돌이 일어나는 지 확인합니다.
            // 스택에 충돌은 게임오버를 뜻합니다.
            if (activePiece2->StackCollision(stackCells2)) {
                gameOver2 = true;
            }
        }
    }

}

HRESULT Engine::Draw()
{
    // 그림이 그려지는 부분입니다.
    HRESULT hr;

    // 그림 그릴 붓 만들고 색상을 정하는 부분입니다.
    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	

	// 게임 내의 각 요소들(스택, 활성 블럭, 대기 블럭)을 실질적으로 그리는 부분입니다.
    // Engine 에서 바로 그리는게 아니라 각 Stack 과 Piece에서 선언된 Draw를 호출해 그립니다.
    stack->Draw(m_pRenderTarget);
    if (gameOver != true) {
        activePiece->Draw(m_pRenderTarget);
    }
    waitingPiece->Draw(m_pRenderTarget);

    stack2->Draw2(m_pRenderTarget);
    if (gameOver2 != true) {
        activePiece2->Draw2(m_pRenderTarget);
    }
    waitingPiece2->Draw2(m_pRenderTarget);

    // 바로 아래의 글씨와 점수를 그리는 부분을 호출해
    // 글씨와 각 점수를 그려줍니다.
    DrawTextAndScore();

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}

void Engine::DrawTextAndScore()
{
    // 글씨와 점수를 그리는 부분입니다.

    // 위치 조정용 변수들
    int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
    int centerRight = RESOLUTION_X - (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;

    // RECT_F를 이용해 글씨 상자를 만들고, 해당 크기만큼 생성된 글씨 상자안에 글씨가 적힙니다.
    // ""안의 내용(Next Piece)들을 그리는데 바로 아래있는 숫자(15)만큼 그립니다.
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

    //실 스코어가 표시되는 부분입니다.
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

    //각 아이템을 소지했는지, 사용했는지 표시하는 부분입니다.
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

    //게임 오버시 나타나는 부분입니다.
    if (over == true) {
        D2D1_RECT_F Over = D2D1::RectF(RESOLUTION_X / 2 - 50, RESOLUTION_Y / 2 + 50, RESOLUTION_X / 2 + 50, RESOLUTION_Y / 2 - 50);
        m_pRenderTarget->DrawText(
            L"Game Over!!",
            15,
            m_pTextFormat,
            Over,
            m_pWhiteBrush
        );
    }
}
