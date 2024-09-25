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
    // �������Դϴ�.
	// Engine.cpp �ȿ��� ���Ǵ� �������� �����ϴ� �����Դϴ�.

    srand(time(NULL));
	
    stack = new Stack();
    stack2 = new Stack();

    activePiece = new Piece();
    activePiece->Activate();
    waitingPiece = new Piece();

    activePiece2 = new Piece();
    activePiece2->Activate();
    waitingPiece2 = new Piece();

    // autoFall �ڵ����� ���� �������� �ӵ�.
    // 0.7 �� �⺻���Դϴ�.
    autoFallDelay = 100;
    autoFallAccumulated = 0;
    keyPressDelay = 0.07;
    keyPressAccumulated = 0;

    autoFallDelay2 = 0.7;
    autoFallAccumulated2 = 0;
    keyPressDelay2 = 0.07;
    keyPressAccumulated2 = 0;

    // ������ ����� Ȯ���ϴ� ����
    fcheck = 0;
    scheck = 0;
}

Engine::~Engine()
{
    // ����

    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);

    // ������� ������ �ٽ� ����
    delete stack;
    delete waitingPiece;
    delete activePiece;
    delete stack2;
    delete waitingPiece2;
    delete activePiece2;
}

HRESULT Engine::InitializeD2D(HWND m_hwnd)
{
    // Direct 2D�� �����ϴ� �κ��Դϴ�.
    D2D1_SIZE_U size = D2D1::SizeU(RESOLUTION_X, RESOLUTION_Y);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &m_pRenderTarget
    );

    // Dirext 2D���� ���Ǵ� �������� �̿��ϴ� �κ��Դϴ�.
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
    // �۾� ��Ʈ�� �����ϴ� �κ��Դϴ�.
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
    // Ű���带 �����ٰ� ���� �� �Դϴ�.
    // �Է��� ������ Ÿ�̹��̴ϱ� �������� false�� �ٲٴ°� Ȯ���Ҽ� �ֽ��ϴ�.

    // 1p�� �̵�
    if (wParam == VK_DOWN)
        downPressed2 = false;

    if (wParam == VK_LEFT)
        leftPressed2 = false;

    if (wParam == VK_RIGHT)
        rightPressed2 = false;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = false;

    // 2p�� �̵�, ���ڵ��� �ƽ�Ű�ڵ�� W,A,S,D �Դϴ�.
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
	// Ű�� �������� �Դϴ�.
    // ���������ϱ� �������� true�� �ٲ�°� Ȯ���Ҽ� �ֽ��ϴ�.

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
    
    // �׽�Ʈ�� ������ ȹ��
    // ���� ����Ű ���� Home, End, Insert, Delete
    if (wParam == VK_HOME)
        ItemGet = true;

    if (wParam == VK_END)
        ItemGet2 = true;

    if (wParam == VK_INSERT)
        SItemGet = true;

    if (wParam == VK_DELETE)
        SItemGet2 = true;

    // ������ ��� Ȯ��
    // 1p�� Ű���� ��� 1, 2
    if (wParam == 49 && ItemGet)
        ItemUse = 1;

    if (wParam == 50 && SItemGet)
        ItemUse = 2;

    //2p�� Ű���� ���� 1, 2
    if (wParam == VK_NUMPAD1 && ItemGet2)
        ItemUse2 = 1;

    if (wParam == VK_NUMPAD2 && SItemGet2)
        ItemUse2 = 2;
}

// ���콺 ���� �Լ����ε� ������� ���� �����
// ȭ���Ȱ�� �ҰŸ� �̰� �ְ� ����ȭ���̳� ������ Ŭ���ǰ� �ϴ¿�
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
    // ����
    // �������� �ý��� �������� �κ��Դϴ�.
    // ���� ���̴� �ý��۵� ��κ��� ���⼭ �������ٰ� �����ص� �����ϴ�.

    // ���ӿ��� üũ
    // �� �� �ϳ��� ���ӿ����� over�� true�� �ٲٰ� �����մϴ�.
    if (gameOver || gameOver2) 
    {
        over = true;

        return;
    }

    // Stack�� Metrix�� �����ؼ� �������� ����� ���ϴ�. (��Ʈ���� ���̴� �κ�)
    Matrix* stackCells = stack->GetCells();
    Matrix* stackCells2 = stack2->GetCells();

    // Ű���带 ������ ���Ƿ� �ϰ���Ű�� �ڵ�
    keyPressAccumulated += elapsedTime;
    if (keyPressAccumulated > keyPressDelay)
    {
        keyPressAccumulated = 0;
        
        // �� �̵� ����� ����������, �̵��� �������� Ȯ���մϴ�.
        if (leftPressed || rightPressed || spacePressed)
        {
            // �̵��� �Ұ��� = ���� ���� ������
            // ���� ���� ������� RemoveLines�� ȣ���� ���� ���� �����մϴ�.
            int removed = stack->RemoveLines(stackCells);
            if (removed > 0)
            {
                //���� �����Ǹ� �� �ٴ� 200�� �� ȹ���ϰ�, �ڵ����� ���� �������� �ӵ��� ������ŵ�ϴ�.
                //autoFallDelay�� ���������� ���� ������
                score += pow(2, removed) * 100;
                autoFallDelay = autoFallDelay * 0.98;
            }
        }

        // ����, ������ �Է� Ȯ���� �� �Լ� ȣ���� �̵�
        if (leftPressed)
            activePiece->GoLeft(stackCells);
        if (rightPressed)
            activePiece->GoRight(stackCells);

        // �� ������
        if (spacePressed)
        {
            activePiece->Rotate(stackCells);
            spacePressed = false;
        }

        // �ϰ�
        // ���� ������ �ִ°� �ƴϰ� �׳� AutoFall�� ���ӽ��Ѽ� ������ ����Ʈ���� ����Դϴ�.
        if (downPressed)
            autoFallAccumulated = autoFallDelay + 1;
    }
    
    //2p���Դϴ�, ���δ� 1p���̶� �����ϴ�.
    keyPressAccumulated2 += elapsedTime;
    if (keyPressAccumulated2 > keyPressDelay2)
    {
        keyPressAccumulated2 = 0;

        // �� �̵� ����� ����������, �̵��� �������� Ȯ���մϴ�.
        if (leftPressed2 || rightPressed2 || spacePressed2)
        {
            // �̵��� �Ұ��� = ���� ���� ������
            // ���� ���� ������� RemoveLines�� ȣ���� ���� ���� �����մϴ�.
            int removed = stack->RemoveLines2(stackCells2);
            if (removed > 0)
            {
                //���� �����Ǹ� �� �ٴ� 200�� �� ȹ���ϰ�, �ڵ����� ���� �������� �ӵ��� ������ŵ�ϴ�.
                //autoFallDelay�� ���������� ���� ������
                score += pow(2, removed) * 100;
                autoFallDelay2 = autoFallDelay2 * 0.98;
            }
        }

        // ����, ������ �Է� Ȯ���� �� �Լ� ȣ���� �̵�
        if (leftPressed2)
            activePiece2->GoLeft(stackCells2);
        if (rightPressed2)
            activePiece2->GoRight(stackCells2);

        // �� ������
        if (spacePressed2)
        {
            activePiece2->Rotate(stackCells2);
            spacePressed2 = false;
        }

        // �ϰ�
        // ���� ������ �ִ°� �ƴϰ� �׳� AutoFall�� ���ӽ��Ѽ� ������ ����Ʈ���� ����Դϴ�.
        if (downPressed2)
            autoFallAccumulated2 = autoFallDelay2 + 1;
    }


    // ���� �ڵ����� �������� �κ��Դϴ�.
    // ���� ��ĭ ������������ �� �ڵ尡 �ѹ� ����ȰŶ�� �����ϸ� �˴ϴ�.
    autoFallAccumulated += elapsedTime;
    if (autoFallAccumulated > autoFallDelay)
    {
       autoFallAccumulated = 0;

        // RemoveLines�� ȣ���� ����� �ִ� ���� �ִ��� Ȯ���մϴ�.
        int removed = stack->RemoveLines(stackCells);
        if (removed > 0)
        {
            //���� ���������� �������� ������ ������Ű�� �ӵ��� ������ŵ�ϴ�.
            score += pow(2, removed) * 100;
            autoFallDelay = autoFallDelay * 0.98;

            //������ 2000���� �����Ҷ����� �������� ȹ���մϴ�.
            if ((score - scorecheck) >= 2000) 
            {
                scorecheck += 2000;

                //������ ���� �ҷ����� ���� ���� �Լ����Դϴ�.
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(1, 100);

                int random = dis(gen) % 2;

                //�������� 2�� ���� �������� �ӵ������� �԰ų� ���ٻ����� �Խ��ϴ�.
                if (random == 0) {
                    ItemGet = true;
                }
                else {
                    SItemGet = true;
                }
            }
        }

        // ����� �ӵ����� ������ ����� Ȯ���մϴ�.
        if (ItemUse2 == 2) {
            SItemGet2 = false;
            
            // �ӵ��� ������Ű�� �κ��Դϴ�.
            autoFallDelay = autoFallDelay - 0.6;

            // �������� ���Ǿ����� ��������� ������ �ٲߴϴ�.
            ItemUse2 = 3;
        }

        // �ӵ������� ������� �κ��Դϴ�.
        // 1p�� �� 3���� ����Ʈ���� ����ϴ�.
        if (fcheck == 3 && ItemUse2 == 3) {
            ItemUse2 = 0;
            fcheck = 0;

            autoFallDelay2 = autoFallDelay2 + 0.6;
        }

        // Advance �Լ��� ȣ���� �ڵ����� ���� ����Ʈ���� �κ��Դϴ�.
        bool isConflict = activePiece->Advance(stackCells);

        // ���� ���������� �� ������ ���� ���ÿ� �״� �κ��Դϴ�.
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

            // �������� ���Ǹ� ���� �������� �� Ȯ���� �������� ���ð��� üũ�մϴ�.
            if (ItemUse2 == 3)
                fcheck++;

            // ���� ���ÿ� �׾����� ����� ���� �����ϰ�
            // ���� ���� ������ ��� ������ ����,
            // ���� ���� ���ο� ���� ����� �ֽ��ϴ�.
            delete activePiece;
            activePiece = waitingPiece;
            activePiece->Activate();
            waitingPiece = new Piece();
            waitingPiece->InitializeD2D(m_pRenderTarget);

            // Ȱ�� ���� ���ð� �ٷ� �浹�� �Ͼ���� Ȯ���մϴ�.
            // Ȱ�� ���ڸ��� ���ð� �浹�� �Ͼ�ű� ������ �� ���� ������ ���մϴ�.
            if (activePiece->StackCollision(stackCells))
                gameOver = true;
        }
    }

    // 2p �����Դϴ�.
    autoFallAccumulated2 += elapsedTime;
    if (autoFallAccumulated2 > autoFallDelay2) 
    {
        autoFallAccumulated2 = 0;

        // RemoveLines�� ȣ���� ����� �ִ� ���� �ִ��� Ȯ���մϴ�.
        int removed = stack->RemoveLines2(stackCells2);
        if (removed > 0)
        {
            //���� ���������� �������� ������ ������Ű�� �ӵ��� ������ŵ�ϴ�.
            score += pow(2, removed) * 100;
            autoFallDelay2 = autoFallDelay2 * 0.98;

            //������ 2000���� �����Ҷ����� �������� ȹ���մϴ�.
            if ((score - scorecheck) >= 2000) {
                scorecheck += 2000;

                //������ ���� �ҷ����� ���� ���� �Լ����Դϴ�.
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<int> dis(1, 100);

                int random = dis(gen) % 2;

                //�������� 2�� ���� �������� �ӵ������� �԰ų� ���ٻ����� �Խ��ϴ�.
                if (random == 0) {
                    ItemGet2 = true;
                }
                else {
                    SItemGet2 = true;
                }
            }
        }

        // ����� �ӵ����� ������ ����� Ȯ���մϴ�.
        if (ItemUse == 2) {
            SItemGet = false;

            // �ӵ��� ������Ű�� �κ��Դϴ�.
            autoFallDelay2 = autoFallDelay2 - 0.6;

            // �������� ���Ǿ����� ��������� ������ �ٲߴϴ�.
            ItemUse = 3;
        }

        // �ӵ������� ������� �κ��Դϴ�.
        // 1p�� �� 3���� ����Ʈ���� ����ϴ�.
        if (scheck == 3 && ItemUse == 3) {
            ItemUse = 0;
            scheck = 0;

            autoFallDelay2 = autoFallDelay2 + 0.6;
        }

        // Advance �Լ��� ȣ���� �ڵ����� ���� ����Ʈ���� �κ��Դϴ�.
        bool isConflict = activePiece2->Advance(stackCells2);

        // ���� ���������� �� ������ ���� ���ÿ� �״� �κ��Դϴ�.
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

            // �������� ���Ǹ� ���� �������� �� Ȯ���� �������� ���ð��� üũ�մϴ�.
            if (ItemUse == 3)
                scheck++;

            // ���� ���ÿ� �׾����� ����� ���� �����ϰ�
            // ���� ���� ������ ��� ������ ����,
            // ���� ���� ���ο� ���� ����� �ֽ��ϴ�.
            delete activePiece2;
            activePiece2 = waitingPiece2;
            activePiece2->Activate();
            waitingPiece2 = new Piece();
            waitingPiece2->InitializeD2D(m_pRenderTarget);

            // ���ÿ� �浹�� �Ͼ�� �� Ȯ���մϴ�.
            // ���ÿ� �浹�� ���ӿ����� ���մϴ�.
            if (activePiece2->StackCollision(stackCells2)) {
                gameOver2 = true;
            }
        }
    }

}

HRESULT Engine::Draw()
{
    // �׸��� �׷����� �κ��Դϴ�.
    HRESULT hr;

    // �׸� �׸� �� ����� ������ ���ϴ� �κ��Դϴ�.
    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	

	// ���� ���� �� ��ҵ�(����, Ȱ�� ��, ��� ��)�� ���������� �׸��� �κ��Դϴ�.
    // Engine ���� �ٷ� �׸��°� �ƴ϶� �� Stack �� Piece���� ����� Draw�� ȣ���� �׸��ϴ�.
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

    // �ٷ� �Ʒ��� �۾��� ������ �׸��� �κ��� ȣ����
    // �۾��� �� ������ �׷��ݴϴ�.
    DrawTextAndScore();

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}

void Engine::DrawTextAndScore()
{
    // �۾��� ������ �׸��� �κ��Դϴ�.

    // ��ġ ������ ������
    int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
    int centerRight = RESOLUTION_X - (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;

    // RECT_F�� �̿��� �۾� ���ڸ� �����, �ش� ũ�⸸ŭ ������ �۾� ���ھȿ� �۾��� �����ϴ�.
    // ""���� ����(Next Piece)���� �׸��µ� �ٷ� �Ʒ��ִ� ����(15)��ŭ �׸��ϴ�.
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

    //�� ���ھ ǥ�õǴ� �κ��Դϴ�.
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

    //�� �������� �����ߴ���, ����ߴ��� ǥ���ϴ� �κ��Դϴ�.
    D2D1_RECT_F TestView = D2D1::RectF(20, 20, 100, 100);
    if (ItemGet == true) {
        m_pRenderTarget->DrawText(
            L"���ٻ��� ������",
            8,
            m_pTextFormat,
            TestView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F TestView2 = D2D1::RectF(720, 20, 800, 100);
    if (ItemGet2 == true) {
        m_pRenderTarget->DrawText(
            L"���ٻ��� ������",
            8,
            m_pTextFormat,
            TestView2,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeView = D2D1::RectF(20, 120, 100, 150);
    if (SItemGet == true) {
        m_pRenderTarget->DrawText(
            L"�ӵ����� ������",
            8,
            m_pTextFormat,
            SpeView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeView2 = D2D1::RectF(720, 120, 800, 150);
    if (SItemGet2 == true) {
        m_pRenderTarget->DrawText(
            L"�ӵ����� ������",
            8,
            m_pTextFormat,
            SpeView2,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F UseView = D2D1::RectF(170, 550, 350, 650);
    if (ItemUse == 1) {
        m_pRenderTarget->DrawText(
            L"������ ����",
            7,
            m_pTextFormat,
            UseView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F UseView2 = D2D1::RectF(570, 550, 750, 650);
    if (ItemUse2 == 1) {
        m_pRenderTarget->DrawText(
            L"������ ����",
            7,
            m_pTextFormat,
            UseView2,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeedView = D2D1::RectF(130, 620, 350, 670);
    if (ItemUse2 == 2 || ItemUse2 == 3) {
        m_pRenderTarget->DrawText(
            L"�ӵ����� ����",
            8,
            m_pTextFormat,
            SpeedView,
            m_pWhiteBrush
        );
    }

    D2D1_RECT_F SpeedView2 = D2D1::RectF(550, 620, 750, 670);
    if (ItemUse == 2 || ItemUse == 3) {
        m_pRenderTarget->DrawText(
            L"�ӵ����� ����",
            8,
            m_pTextFormat,
            SpeedView2,
            m_pWhiteBrush
        );
    }

    //���� ������ ��Ÿ���� �κ��Դϴ�.
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
