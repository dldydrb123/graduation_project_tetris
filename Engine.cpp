#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Piece.h"
#include "Engine.h"
#include "Item.h"
#include <wincodec.h>
#include <wrl.h> // For Microsoft::WRL::ComPtr

using namespace Microsoft::WRL;

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Windowscodecs.lib")

#define SOLO_PLAY 1

// ���� ����
ComPtr<ID2D1Factory> d2dFactory;
ComPtr<IWICImagingFactory> wicFactory;
ComPtr<ID2D1RenderTarget> renderTarget;

// DRAW �Լ� ������ ����� �ڵ�
HRESULT Engine::DrawJpgImage(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pWICFactory, const wchar_t* filename, float x, float y, float width, float height)
{
    HRESULT hr = S_OK;
    ComPtr<IWICBitmapDecoder> pDecoder;
    ComPtr<IWICBitmapFrameDecode> pFrame;
    ComPtr<IWICFormatConverter> pConverter;
    ComPtr<ID2D1Bitmap> pBitmap;

    // JPG �̹����� ���ڵ�
    hr = pWICFactory->CreateDecoderFromFilename(
        filename,                   // �̹��� ���� ���
        nullptr,                    // GUID (�ڵ� ����)
        GENERIC_READ,               // �б� ���� ���
        WICDecodeMetadataCacheOnLoad,   // ��ü ��Ÿ�����͸� �ε�
        &pDecoder                   // ��� ���ڴ�
    );
    if (FAILED(hr)) {
        return hr;
    }

    // �̹����� ù ��° �������� �����ɴϴ�.
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        return hr;
    }

    // ���� ��ȯ�⸦ �����մϴ�.
    hr = pWICFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) {
        return hr;
    }

    // ��Ʈ�� ������ 32bppPBGRA�� ��ȯ�մϴ�. (Direct2D�� ȣȯ)
    hr = pConverter->Initialize(
        pFrame.Get(),                   // ���� ��Ʈ�� �ҽ�
        GUID_WICPixelFormat32bppPBGRA,  // ��ȯ�� ����
        WICBitmapDitherTypeNone,        // ����� ����
        nullptr,                        // �ȷ�Ʈ ����
        0.f,                            // ���� �Ӱ谪 (����)
        WICBitmapPaletteTypeCustom      // ����� ���� �ȷ�Ʈ
    );
    if (FAILED(hr)) {
        return hr;
    }

    // WIC ��Ʈ���� Direct2D ��Ʈ������ ��ȯ
    hr = pRenderTarget->CreateBitmapFromWicBitmap(
        pConverter.Get(),
        nullptr,
        &pBitmap
    );
    if (FAILED(hr)) {
        return hr;
    }

    // ��Ʈ���� ���� Ÿ�ٿ� �׸��ϴ�. ������ ��ǥ ���� ����� �׸� ��ġ�� �����մϴ�.
    pRenderTarget->DrawBitmap(
        pBitmap.Get(),
        D2D1::RectF(x, y, x + width, y + height)  // ������ ��ġ�� ũ��
    );

    return hr;
}
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
#if (SOLO_PLAY!=1)
    autoFallDelay = 0.7;
#else
    autoFallDelay = 100;
#endif
    autoFallAccumulated = 0;
    keyPressDelay = 0.07;
    keyPressAccumulated = 0;

    autoFallDelay2 = 0.7;
    autoFallAccumulated2 = 0;
    keyPressDelay2 = 0.07;
    keyPressAccumulated2 = 0;

    fcheck = 0;
    scheck = 0;

    // ������ ����� Ȯ���ϴ� ����
    blindcheck = 0;
    blindcheck2 = 0;

    //��� �ϰ��� ����
    fall = 0;
    fall2 = 0;
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

    // Direct2D ���丮 ����
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf());
    if (FAILED(hr)) {
        // ���� ó��
    }

    // WIC ���丮 ����
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );
    if (FAILED(hr)) {
        // ���� ó��
    }

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

    // 2p�� �̵�
    if (wParam == VK_DOWN)
        downPressed2 = false;

    if (wParam == VK_LEFT)
        leftPressed2 = false;

    if (wParam == VK_RIGHT)
        rightPressed2 = false;

    if (wParam == VK_UP)
        spacePressed2 = false;

    // 1p�� �̵�, ���ڵ��� �ƽ�Ű�ڵ�� W,A,S,D �Դϴ�.
    if (wParam == 83)
        downPressed = false;

    if (wParam == 65)
        leftPressed = false;

    if (wParam == 68)
        rightPressed = false;

    if (wParam == 87)
        spacePressed = false;
}

void Engine::KeyDown(WPARAM wParam)
{
	// Ű�� �������� �Դϴ�.
    // ���������ϱ� �������� true�� �ٲ�°� Ȯ���Ҽ� �ֽ��ϴ�.

    // 2p
    if (wParam == VK_DOWN)
        downPressed2 = true;

    if (wParam == VK_LEFT)
        leftPressed2 = true;

    if (wParam == VK_RIGHT)
        rightPressed2 = true;

    if (wParam == VK_UP)
        spacePressed2 = true;

    if (wParam == VK_RETURN)
        enteringPressed2 = true;

    // 1p
    if (wParam == 83)
        downPressed = true;

    if (wParam == 65)
        leftPressed = true;

    if (wParam == 68)
        rightPressed = true;

    if (wParam == 87)
        spacePressed = true;

    if (wParam == VK_SPACE)
        enteringPressed = true;

    // �׽�Ʈ�� ������ ȹ��
    // ���� ����Ű ���� Home, End, Insert, Delete
    if (wParam == VK_HOME)
    {
        Itemarr2[0]++;
        Itemarr2[1]++;
        Itemarr2[2]++;
        Itemarr2[3]++;
        Itemarr2[4]++;
        Itemarr2[5]++;
    }

    if (wParam == VK_END);

    if (wParam == VK_INSERT)
    {
        Itemarr[0]++;
        Itemarr[1]++;
        Itemarr[2]++;
        Itemarr[3]++;
        Itemarr[4]++;
        Itemarr[5]++;
    }

    if (wParam == VK_DELETE);

    // ������ ��� Ȯ��
    // 1p�� Ű���� ��� 1 ~ 6
    if (wParam == 49 && Itemarr[0] > 0)
    {
        item1_1 = true;
        Itemarr[0]--;
    }
    if (wParam == 50 && Itemarr[1] > 0)
    {
        item1_2 = true;
        Itemarr[1]--;
    }
    if (wParam == 51 && Itemarr[2] > 0)
    {
        item1_3 = true;
        Itemarr[2]--;
    }
    if (wParam == 52 && Itemarr[3] > 0)
    {
        item1_4 = true;
        Itemarr[3]--;
    }
    if (wParam == 53 && Itemarr[4] > 0)
    {
        item1_5 = true;
    }
    if (wParam == 54 && Itemarr[5] > 0)
    {
        item1_6 = true;
        Itemarr[5]--;
    }
    //2p�� Ű���� ���� 1 ~ 6
    if (wParam == VK_NUMPAD1 && Itemarr2[0] > 0)
    {
        item2_1 = true;
        Itemarr2[0]--;
    }
    if (wParam == VK_NUMPAD2 && Itemarr2[1] > 0)
    {
        item2_2 = true;
        Itemarr2[1]--;
    }
    if (wParam == VK_NUMPAD3 && Itemarr2[2] > 0)
    {
        item2_3 = true;
        Itemarr2[2]--;
    }
    if (wParam == VK_NUMPAD4 && Itemarr2[3] > 0)
    {
        item2_4 = true;
    }
    if (wParam == VK_NUMPAD5 && Itemarr2[4] > 0)
    {
        item2_5 = true;
    }
    if (wParam == VK_NUMPAD6 && Itemarr2[5] > 0)
    {
        item2_6 = true;
        Itemarr2[5]--;
    }
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

    if (item1_4) {
        delete waitingPiece;
        changePiece = new Piece();
        waitingPiece = changePiece;
        waitingPiece->InitializeD2D(m_pRenderTarget);
        Itemarr[3]--;
        item1_4 = false;
    }

    if (item2_4) {
        delete waitingPiece2;
        changePiece2 = new Piece();
        waitingPiece2 = changePiece2;
        waitingPiece2->InitializeD2D(m_pRenderTarget);
        Itemarr2[3]--;
        item2_4 = false;
    }

    if (item1_5) {
        delete waitingPiece;
        changePiece = new Piece();
        waitingPiece = changePiece;
        waitingPiece->InitializeD2D(m_pRenderTarget);
        Itemarr[4]--;
        item1_5 = false;
    }

    if (item2_5) {
        delete waitingPiece2;
        changePiece2 = new Piece();
        waitingPiece2 = changePiece2;
        waitingPiece2->InitializeD2D(m_pRenderTarget);
        Itemarr2[4]--;
        item2_5 = false;
    }

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
                if (autoFallDelay > 0.175) {
                    autoFallDelay = autoFallDelay * 0.98;
                }
                else {
                    autoFallDelay = 0.175;
                }
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

        // �� ��� �ϰ��� ���� �ӵ��� ������ŵ�ϴ�.
        if (enteringPressed) {
            autoFallDelay = 0.001;
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
                score2 += pow(2, removed) * 100;
                if (autoFallDelay2 > 0.175) {
                    autoFallDelay2 = autoFallDelay2 * 0.98;
                }
                else {
                    autoFallDelay2 = 0.175;
                }

                
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

        // �� ��� �ϰ��� ���� �ӵ��� ������ŵ�ϴ�.
        if (enteringPressed2) {
            autoFallDelay2 = 0.001;
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
            if (autoFallDelay > 0.175) {
                autoFallDelay = autoFallDelay * 0.98;
            }
            else {
                autoFallDelay = 0.175;
            }

            switch (ItemGet)
            {
            case 1:
                Itemarr[0] += 1;
                break;
            case 2:
                Itemarr[1] += 1;
                break;
            case 3:
                Itemarr[2] += 1;
                break;
            case 4:
                Itemarr[3] += 1;
                break;
            case 5:
                Itemarr[4] += 1;
                break;
            case 6:
                Itemarr[5] += 1;
                break;
            }
        }

        // Advance �Լ��� ȣ���� �ڵ����� ���� ����Ʈ���� �κ��Դϴ�.
        bool isConflict = activePiece->Advance(stackCells);

        int brushIndex = activePiece->GetRandomBrushIndex();
        // ���� ���������� �� ������ ���� ���ÿ� �״� �κ��Դϴ�.
        if (isConflict)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (activePiece->GetCells()->Get(j, i) == 2)
                    {
                        int realx = activePiece->GetPosition().x + j;
                        int realy = activePiece->GetPosition().y + i;
                        stackCells->Set(realx, realy, 8);
                    }
                    else if (activePiece->GetCells()->Get(j, i) > 0)
                    {
                        int realx = activePiece->GetPosition().x + j;
                        int realy = activePiece->GetPosition().y + i;
                        stackCells->Set(realx, realy, brushIndex);
                    }
                }
            }

            // ����ε� �������� ����� Ȯ���ϰ� ���ӽð��� üũ�մϴ�.
            if (item2_3) {
                blindcheck++;
                Itemarr2[2]--;
                if (blindcheck == 3) {
                    blindcheck = 0;
                    item2_3 = false;
                }
            }

            // ��� �ϰ��� �۵��Ǵ� �κ��Դϴ�.
            // ��� �ϰ��� �ٴڿ� ������ �����ϰ� �ٴڿ� �����ɽ� �ӵ��� ������ ������ �ǵ����ϴ�.
            if (enteringPressed == true)
                fcheck++;

            if (fcheck == 1) {
                autoFallDelay = 0.7;
                fcheck = 0;
                enteringPressed = false;
            }


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
            score2 += pow(2, removed) * 100;
            autoFallDelay2 = autoFallDelay2 * 0.98;
        }

        // Advance �Լ��� ȣ���� �ڵ����� ���� ����Ʈ���� �κ��Դϴ�.
        bool isConflict = activePiece2->Advance(stackCells2);

        int brushIndex = activePiece2->GetRandomBrushIndex();
        // ���� ���������� �� ������ ���� ���ÿ� �״� �κ��Դϴ�.
        if (isConflict)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (activePiece2->GetCells()->Get(j, i) == 3) 
                    {
                        int realx = activePiece2->GetPosition().x + j;
                        int realy = activePiece2->GetPosition().y + i;
                        for (int x = 0; x < 3; x++) {
                            for (int y = 0; y < 3; y++) {
                                if(realy + (y-1) >= STACK_HEIGHT)
                                {
                                    break;
                                }
                                stackCells2->Set(realx + (x-1), realy + (y-1), 0);
                            }
                        }
                    }
                    else if (activePiece2->GetCells()->Get(j, i) == 2)
                    {
                        int realx = activePiece2->GetPosition().x + j;
                        int realy = activePiece2->GetPosition().y + i;
                        stackCells2->Set(realx, realy, 8);
                    }
                    else if (activePiece2->GetCells()->Get(j, i) > 0)
                    {
                        int realx = activePiece2->GetPosition().x + j;
                        int realy = activePiece2->GetPosition().y + i;
                        stackCells2->Set(realx, realy, brushIndex);
                    }
                }
            }

            // ����ε� �������� ����� Ȯ���ϰ� ���ӽð��� üũ�մϴ�.
            if (item1_3) {
                blindcheck2++;
                if (blindcheck2 == 3) {
                    blindcheck2 = 0;
                    item1_3 = false;
                }
            }

            // ��� �ϰ��� �۵��Ǵ� �κ��Դϴ�.
            // ��� �ϰ��� �ٴڿ� ������ �����ϰ� �ٴڿ� �����ɽ� �ӵ��� ������ ������ �ǵ����ϴ�.
            if (enteringPressed2 == true)
                scheck++;

            if (scheck == 1) {
                autoFallDelay2 = 0.7;
                scheck = 0;
                enteringPressed2 = false;
            }
            
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
        // JPG �̹����� ȭ�鿡 �׸��� �κ� - ������ ��ǥ ������ ����
    
    //���ȭ��
    float x = 0.0f;
    float y = 0.0f;
    float width = 900.0f;
    float height = 800.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\tetris wall paper.png", x, y, width, height);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }

    //1p next
    float x2 = 70.0f; //1p next
    float y2 = 140.0f;
    float width2 = 150.0f;
    float height2 = 120.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\next.png", x2, y2, width2, height2);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }

    //1p score
    float x3 = 70.0f;
    float y3 = 380.0f;
    float width3 = 145.0f;
    float height3 = 120.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\score.png", x3, y3, width3, height3);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }


    //2p score
    float x4 = 720.0f;
    float y4 = 380.0f;
    float width4 = 145.0f;
    float height4 = 120.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\score2.png", x4, y4, width4, height4);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }

    //2p next
    float x5 = 720.0f;
    float y5 = 140.0f;
    float width5 = 150.0f;
    float height5 = 120.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\next.png", x5, y5, width5, height5);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }

    //2p black
    float x6 = 538.0f;
    float y6 = 138.0f;
    float width6 = 210.0f;
    float height6 = 491.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\black.png", x6, y6, width6, height6);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }
    //1p black
    float x7 = 194.0f;
    float y7 = 138.0f;
    float width7 = 210.0f;
    float height7 = 491.0f;
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\black.png", x7, y7, width7, height7);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }

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

    //1p Blind Item
    //1p�� ����ϴ� �Ŷ� 2p�� ���带 �����ϴ�.
    if (item1_3) {
        float ix2 = 542.0f;
        float iy2 = 383.5f;
        float iwidth2 = 202.0f;
        float iheight2 = 240.5f;
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\Blind.jpg", ix2, iy2, iwidth2, iheight2);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }
    }

    //2p Blind Item
    //2p�� ����ϴ� �Ŷ� 1p�� ���带 �����ϴ�.
    if (item2_3) {
        float ix = 198.0f;
        float iy = 383.5f;
        float iwidth = 202.0f;
        float iheight = 240.5f;
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\Blind.jpg", ix, iy, iwidth, iheight);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }
    }

    // �ٷ� �Ʒ��� �۾��� ������ �׸��� �κ��� ȣ����
    // �۾��� �� ������ �׷��ݴϴ�.
    DrawTextAndScore();

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}

HRESULT Engine::DrawTextAndScore()
{
    // �۾��� ������ �׸��� �κ��Դϴ�.

    // ��ġ ������ ������
    int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
    int centerRight = RESOLUTION_X - (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;
    int centerLeft = (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;


    // RECT_F�� �̿��� �۾� ���ڸ� �����, �ش� ũ�⸸ŭ ������ �۾� ���ھȿ� �۾��� �����ϴ�.
    // ""���� ����(Next Piece)���� �׸��µ� �ٷ� �Ʒ��ִ� ����(15)��ŭ �׸��ϴ�.
   /* D2D1_RECT_F Next = D2D1::RectF(centerLeft - 200, padding, centerLeft, padding + 100);
    m_pRenderTarget->DrawText(
        L"Next Piece",
        15,
        m_pTextFormat,
        Next,
        m_pWhiteBrush
    );*/


   /* D2D1_RECT_F ScoreLoca = D2D1::RectF(centerLeft - 200, padding + 200, centerLeft, padding + 300);
    m_pRenderTarget->DrawText(
        L"1P Score",
        8,
        m_pTextFormat,
        ScoreLoca,
        m_pWhiteBrush
    );*/

    //�� ���ھ ǥ�õǴ� �κ��Դϴ�.
    D2D1_RECT_F PScore = D2D1::RectF(centerLeft -265, padding + 300, centerLeft+175, padding + 420);
    WCHAR scoreStr[64];
    swprintf_s(scoreStr, L"%d        ", score);
    m_pRenderTarget->DrawText(
        scoreStr,
        7,
        m_pTextFormat,
        PScore,
        m_pWhiteBrush
    );

    /*
    D2D1_RECT_F Next2 = D2D1::RectF(centerRight, padding, centerRight + 200, padding + 100);
    m_pRenderTarget->DrawText(
        L"Next Piece",
        15,
        m_pTextFormat,
        Next2,
        m_pWhiteBrush
    );*/

    /*
    D2D1_RECT_F ScoreLoca2 = D2D1::RectF(centerRight, padding + 200, centerRight + 200, padding + 300);
    m_pRenderTarget->DrawText(
        L"2P Score",
        8,
        m_pTextFormat,
        ScoreLoca2,
        m_pWhiteBrush
    );*/

    //�� ���ھ ǥ�õǴ� �κ��Դϴ�.
    D2D1_RECT_F PScore2 = D2D1::RectF(centerRight, padding + 300, centerRight + 170, padding + 420);
    WCHAR scoreStr2[64];
    swprintf_s(scoreStr2, L"%d        ", score);
    m_pRenderTarget->DrawText(
        scoreStr2,
        7,
        m_pTextFormat,
        PScore2,
        m_pWhiteBrush
    );

    D2D1_RECT_F TestView1_1 = D2D1::RectF(190, 15, 200, 30);
    D2D1_RECT_F TestView1_2 = D2D1::RectF(210, 15, 220, 30);
    D2D1_RECT_F TestView1_3 = D2D1::RectF(230, 15, 240, 30);
    D2D1_RECT_F TestView1_4 = D2D1::RectF(250, 15, 260, 30);
    D2D1_RECT_F TestView1_5 = D2D1::RectF(270, 15, 280, 30);
    D2D1_RECT_F TestView1_6 = D2D1::RectF(290, 15, 300, 30);
    D2D1_RECT_F TestView1_01 = D2D1::RectF(190, 50, 200, 60);
    D2D1_RECT_F TestView1_02 = D2D1::RectF(210, 50, 220, 60);
    D2D1_RECT_F TestView1_03 = D2D1::RectF(230, 50, 240, 60);
    D2D1_RECT_F TestView1_04 = D2D1::RectF(250, 50, 260, 60);
    D2D1_RECT_F TestView1_05 = D2D1::RectF(270, 50, 280, 60);
    D2D1_RECT_F TestView1_06 = D2D1::RectF(290, 50, 300, 60);
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0:
            if (Itemarr[0] > 0) {
                m_pRenderTarget->DrawText(
                    L"1",
                    1,
                    m_pTextFormat,
                    TestView1_1,
                    m_pWhiteBrush
                );
                if (Itemarr[0] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr[0]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView1_01,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 1:
            if (Itemarr[1] > 0) {
                m_pRenderTarget->DrawText(
                    L"2",
                    1,
                    m_pTextFormat,
                    TestView1_2,
                    m_pWhiteBrush
                );
                if (Itemarr[1] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr[1]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView1_02,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 2:
            if (Itemarr[2] > 0) {
                m_pRenderTarget->DrawText(
                    L"3",
                    1,
                    m_pTextFormat,
                    TestView1_3,
                    m_pWhiteBrush
                );
                if (Itemarr[2] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr[2]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView1_03,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 3:
            if (Itemarr[3] > 0) {
                m_pRenderTarget->DrawText(
                    L"4",
                    1,
                    m_pTextFormat,
                    TestView1_4,
                    m_pWhiteBrush
                );
                if (Itemarr[3] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr[3]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView1_04,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 4:
            if (Itemarr[4] > 0) {
                m_pRenderTarget->DrawText(
                    L"5",
                    1,
                    m_pTextFormat,
                    TestView1_5,
                    m_pWhiteBrush
                );
                if (Itemarr[4] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr[4]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView1_05,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 5:
            if (Itemarr[5] > 0) {
                m_pRenderTarget->DrawText(
                    L"6",
                    1,
                    m_pTextFormat,
                    TestView1_6,
                    m_pWhiteBrush
                );
                if (Itemarr[5] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr[5]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView1_06,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        default:
            break;
        }
    }

    D2D1_RECT_F TestView2_1 = D2D1::RectF(550, 15, 560, 30);
    D2D1_RECT_F TestView2_2 = D2D1::RectF(570, 15, 590, 30);
    D2D1_RECT_F TestView2_3 = D2D1::RectF(590, 15, 610, 30);
    D2D1_RECT_F TestView2_4 = D2D1::RectF(610, 15, 630, 30);
    D2D1_RECT_F TestView2_5 = D2D1::RectF(630, 15, 650, 30);
    D2D1_RECT_F TestView2_6 = D2D1::RectF(650, 15, 670, 30);
    D2D1_RECT_F TestView2_01 = D2D1::RectF(550, 50, 560, 60);
    D2D1_RECT_F TestView2_02 = D2D1::RectF(570, 50, 590, 60);
    D2D1_RECT_F TestView2_03 = D2D1::RectF(590, 50, 610, 60);
    D2D1_RECT_F TestView2_04 = D2D1::RectF(610, 50, 630, 60);
    D2D1_RECT_F TestView2_05 = D2D1::RectF(630, 50, 650, 60);
    D2D1_RECT_F TestView2_06 = D2D1::RectF(650, 50, 670, 60);
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0:
            if (Itemarr2[0] > 0) {
                m_pRenderTarget->DrawText(
                    L"1",
                    1,
                    m_pTextFormat,
                    TestView2_1,
                    m_pWhiteBrush
                );
                if (Itemarr2[0] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr2[0]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView2_01,
                        m_pWhiteBrush
                    );
                }

            }
            break;
        case 1:
            if (Itemarr2[1] > 0) {
                m_pRenderTarget->DrawText(
                    L"2",
                    1,
                    m_pTextFormat,
                    TestView2_2,
                    m_pWhiteBrush
                );
                if (Itemarr2[1] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr2[1]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView2_02,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 2:
            if (Itemarr2[2] > 0) {
                m_pRenderTarget->DrawText(
                    L"3",
                    1,
                    m_pTextFormat,
                    TestView2_3,
                    m_pWhiteBrush
                );
                if (Itemarr2[2] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr2[2]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView2_03,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 3:
            if (Itemarr2[3] > 0) {
                m_pRenderTarget->DrawText(
                    L"4",
                    1,
                    m_pTextFormat,
                    TestView2_4,
                    m_pWhiteBrush
                );
                if (Itemarr2[3] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr2[3]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView2_04,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 4:
            if (Itemarr2[4] > 0) {
                m_pRenderTarget->DrawText(
                    L"5",
                    1,
                    m_pTextFormat,
                    TestView2_5,
                    m_pWhiteBrush
                );
                if (Itemarr2[4] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr2[4]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView2_05,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        case 5:
            if (Itemarr2[5] > 0) {
                m_pRenderTarget->DrawText(
                    L"6",
                    1,
                    m_pTextFormat,
                    TestView2_6,
                    m_pWhiteBrush
                );
                if (Itemarr2[5] > 1) {
                    WCHAR scoreStr[64];
                    swprintf_s(scoreStr, L"%d", Itemarr2[5]);
                    m_pRenderTarget->DrawText(
                        scoreStr,
                        2,
                        m_pTextFormat,
                        TestView2_06,
                        m_pWhiteBrush
                    );
                }
            }
            break;
        default:
            break;
        }
    }

    HRESULT hr;
    //���� ������ ��Ÿ���� �κ��Դϴ�.
    if (over == true) {
       /* D2D1_RECT_F Over = D2D1::RectF(RESOLUTION_X / 2 - 50, RESOLUTION_Y / 2 + 50, RESOLUTION_X / 2 + 50, RESOLUTION_Y / 2 - 50);
        m_pRenderTarget->DrawText(
            L"Game Over!!",
            15,
            m_pTextFormat,
            Over,
            m_pWhiteBrush
          
        );*/

        //gameover
        float x7 = 325.0f;
        float y7 = 230.0f;
        float width7 = 300.0f;
        float height7 = 300.0f;
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\gameover.png", x7, y7, width7, height7);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }
    }
}
