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

#define SOLO_PLAY 0 

// ���� ����
ComPtr<ID2D1Factory> d2dFactory;
ComPtr<IWICImagingFactory> wicFactory;
ComPtr<ID2D1RenderTarget> renderTarget;

std::wstring StringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void Engine::SaveScore(const std::string& fileName, int score1, const std::wstring& player1Name, int score2, const std::wstring& player2Name) {
    // std::string�� std::wstring���� ��ȯ
    std::wstring wFileName = StringToWString(fileName);

    // ������ ���ų� ������ ���� ���� (�߰� ���� ����)
    HANDLE hFile = CreateFile(wFileName.c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // ������ ������
    if (hFile != INVALID_HANDLE_VALUE) {
        // ���� ������ �̵�
        SetFilePointer(hFile, 0, NULL, FILE_END);

        // �÷��̾� �̸��� UTF-8 ���ڿ��� ��ȯ
        std::string player1NameStr(player1Name.begin(), player1Name.end());
        std::string player2NameStr(player2Name.begin(), player2Name.end());

        // "score,player1Name,score2,player2Name," �������� ���ڿ� ����
        std::string entry = player1NameStr + "," + std::to_string(score1) + "," +
            player2NameStr + "," + std::to_string(score2) + ",";

        DWORD bytesWritten;
        // �� ��� �߰�
        WriteFile(hFile, entry.c_str(), entry.size(), &bytesWritten, NULL);

        // ���� �ݱ�
        CloseHandle(hFile);
    }
}

int Engine::LoadScore(const std::string& fileName) {
    std::wstring wFileName = StringToWString(fileName);

    HANDLE hFile = CreateFile(wFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }

    char buffer[256];
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        CloseHandle(hFile);
        return -1;
    }

    buffer[bytesRead] = '\0';
    CloseHandle(hFile);

    return std::stoi(buffer);
}

// DRAW �Լ� ������ ����� �ڵ�
HRESULT Engine::DrawJpgImage(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pWICFactory, const wchar_t* filename, float x, float y, float width, float height)
{
    HRESULT hr = S_OK;
    ComPtr<IWICBitmapDecoder> pDecoder;
    ComPtr<IWICBitmapFrameDecode> pFrame;
    ComPtr<IWICFormatConverter> pConverter;
    ComPtr<ID2D1Bitmap> pBitmap;

    // JPG �̹����� ���ڵ�
    if (pWICFactory != NULL) {
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
    }

    // �̹����� ù ��° �������� �����ɴϴ�.
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        return hr;
    }

    // ���� ��ȯ�⸦ �����մϴ�.  
    if (pWICFactory != NULL)
    {
        hr = pWICFactory->CreateFormatConverter(&pConverter);
        if (FAILED(hr)) {
            return hr;
        }
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
    if (pRenderTarget != NULL) {
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
    }
    return hr;
}

Engine::Engine() : m_pDirect2dFactory(NULL), m_pRenderTarget(NULL)
{
    // �������Դϴ�.
	// Engine.cpp �ȿ��� ���Ǵ� �������� �����ϴ� �����Դϴ�.
    m_pWhiteBrush = nullptr;
    m_pDWriteFactory = nullptr;
    m_pTextFormat = nullptr;
    m_pItemBrush = nullptr;

    srand(static_cast<unsigned int>(time(NULL)));
	
    stack = new Stack();
    stack2 = new Stack();

    activePiece = new Piece();
    activePiece->Activate();
    waitingPiece = new Piece();
    changePiece = new Piece();
    /*shadowPiece = new Piece();
    shadowPiece->Shadow();*/

    activePiece2 = new Piece();
    activePiece2->Activate();
    waitingPiece2 = new Piece();
    changePiece2 = new Piece();
    /*shadowPiece2 = new Piece();
    shadowPiece2->Shadow();*/

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

    // ���� Ȯ�ο� ����
    fcheck = 0;
    scheck = 0;

    // ����ε� �����ۿ� ����
    height = 0;
    height2 = 0;

    // ������ ����� Ȯ���ϴ� ����
    blindcheck = 0;
    blindcheck2 = 0;

    // ��� �ϰ��� ����
    fall = 0;
    fall2 = 0; 

    // ��� �ϰ� �����ۿ� ����
    itemfall = 0;
    itemfall2 = 0; 
}

Engine::~Engine()
{

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

    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Purple),
        &m_pItemBrush
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


    if (wParam == VK_NUMPAD0)
    {
    }
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

    // ������ ��� Ȯ��
    // 1p�� Ű���� ��� 1 ~ 6
    if (wParam == 49 && ItemGet > 0)
    {
        switch (ItemGet)
        {
        case 1:
            item1_1 = true;
            break;
        case 2:
            item1_2 = true;
            break;
        case 3:
            item1_3 = true;
            if (Itemarr[2] > 0) {
                ItemGet = 0;
                Itemarr[2]--;
            }
            break;
        case 4:
            item1_4 = true;
            break;
        case 5:
            item1_5 = true;
            break;
        case 6:
            item1_6 = true;
            break;
        }
    }
     
    //2p�� Ű���� ���� 1 ~ 6
    if (wParam == VK_NUMPAD1 && ItemGet2 > 0)
    {
        switch (ItemGet2)
        {
        case 1:
            item2_1 = true;
            break;
        case 2:
            item2_2 = true;
            break;
        case 3:
            item2_3 = true;
            if (Itemarr2[2] > 0) {
                Itemarr2[2]--;
                ItemGet2 = 0;
            }
            break;
        case 4:
            item2_4 = true;
            break;
        case 5:
            item2_5 = true;
            break;
        case 6:
            item2_6 = true;
            break;
        }
    }

    if (wParam == VK_NUMPAD0)
    {
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
    if (Gover) 
    {
        const std::string fileName = "score.txt";
        // ���� ����
        SaveScore("score.txt", score, player1Name, score2, player2Name);
        scoreSaved = true;

        return;
    }

    // Stack�� Metrix�� �����ؼ� �������� ����� ���ϴ�. (��Ʈ���� ���̴� �κ�)
    Matrix* stackCells = stack->GetCells();
    Matrix* changeCells = stack->GetCells();
    Matrix* stackCells2 = stack2->GetCells();
    Matrix* changeCells2 = stack2->GetCells();

    // 4�� ������ ���Ǵ� �κ�
    if (item1_4) {
        delete waitingPiece;
        changePiece = new Piece();
        waitingPiece = changePiece;
        waitingPiece->InitializeD2D(m_pRenderTarget);
        Itemarr[3]--;
        item1_4 = false;
        ItemGet = 0;
    }

    if (item2_4) {
        delete waitingPiece2;
        changePiece2 = new Piece();
        waitingPiece2 = changePiece2;
        waitingPiece2->InitializeD2D(m_pRenderTarget);
        Itemarr2[3]--;
        item2_4 = false;
        ItemGet2 = 0;
    }
    
    // 5�� ������ ���Ǵ� �κ�
    if (item1_5) {
        delete waitingPiece;
        changePiece = new Piece();
        waitingPiece = changePiece;
        waitingPiece->InitializeD2D(m_pRenderTarget);
        Itemarr[4]--;
        item1_5 = false;
        ItemGet = 0;
    }

    if (item2_5) {
        delete waitingPiece2;
        changePiece2 = new Piece();
        waitingPiece2 = changePiece2;
        waitingPiece2->InitializeD2D(m_pRenderTarget);
        Itemarr2[4]--;
        item2_5 = false;
        ItemGet2 = 0;
    }


    // 6�� ������ ���Ǵ� �κ�
    if (item1_6) {
        for (int i = STACK_HEIGHT - 1; i >= 0; i--) {
            for (int j = STACK_WIDTH; j >= 0; j--) {
                if (j == 0) {
                    stackCells2->Set(j, i, changeCells2->Get(10, i));
                }
                else {
                    stackCells2->Set(j, i, stackCells2->Get(j - 1, i));
                }
            }
        }
        item1_6 = false;
        Itemarr[5]--;
        ItemGet = 0;
    }

    if (item2_6) {
        for (int i = STACK_HEIGHT - 1; i >= 0; i--) {
            for (int j = STACK_WIDTH; j >= 0; j--) {
                if (j == 0) {
                    stackCells->Set(j, i, changeCells->Get(10, i));
                }
                else {
                    stackCells->Set(j, i, stackCells->Get(j - 1, i));
                }
            }
        }
        item2_6 = false;
        Itemarr2[5]--;
        ItemGet2 = 0;
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
            if (dualcheck == false) {
                fall = autoFallDelay;
            }
            autoFallDelay = 0.001;
            dualcheck = true;
        }

        // 2�� �������� ����� ������ ���� ��ó��� ��ŵ�ϴ�.
        if (item1_2) {
            if (fallcheck2 == false) {
                itemfall2 = autoFallDelay2;
            }
            autoFallDelay2 = 0.001;
            fallcheck2 = true;
            ItemGet = 0;
            Itemarr[1]--;
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
            if (dualcheck2 == false) {
                fall2 = autoFallDelay2;
            }
            autoFallDelay2 = 0.001;
            dualcheck2 = true;
        }

        // 2�� �������� ����� ������ ���� ��ó��� ��ŵ�ϴ�.
        if(item2_2){
            if (fallcheck == false) {
                itemfall = autoFallDelay;
            }
            autoFallDelay = 0.001;
            fallcheck = true;
            ItemGet2 = 0;
            Itemarr2[1]--;
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
                    if (activePiece->GetCells()->Get(j, i) == 3)
                    {
                        int realx = static_cast<int>(activePiece->GetPosition().x + j);
                        int realy = static_cast<int>(activePiece->GetPosition().y + i);
                        for (int x = 0; x < 3; x++) {
                            for (int y = 0; y < 3; y++) {
                                if (realy + (y - 1) >= STACK_HEIGHT)
                                {
                                    break;
                                }
                                stackCells->Set(realx + (x - 1), realy + (y - 1), 0);
                            }
                        }
                    }
                    else if (activePiece->GetCells()->Get(j, i) == 2)
                    {
                        int realx = static_cast<int>(activePiece->GetPosition().x + j);
                        int realy = static_cast<int>(activePiece->GetPosition().y + i);
                        stackCells->Set(realx, realy, 8);
                    }
                    else if (activePiece->GetCells()->Get(j, i) > 0)
                    {
                        int realx = static_cast<int>(activePiece->GetPosition().x + j);
                        int realy = static_cast<int>(activePiece->GetPosition().y + i);
                        stackCells->Set(realx, realy, brushIndex);
                    }
                }
            }

            // ����ε� �������� ����� Ȯ���ϰ� ���ӽð��� üũ�մϴ�.
            if (item2_3) {
                blindcheck++;
                if (blindcheck == 3) {
                    blindcheck = 0;
                    item2_3 = false;
                }
            }

            // ���� �������� ����� Ȯ��, �ӵ��� �ٽ� ������ŵ�ϴ�.
            if (item2_2) {
                autoFallDelay = itemfall;
                item2_2 = false;
                fallcheck = false;
            }

            // ��� �ϰ��� �۵��Ǵ� �κ��Դϴ�.
            // ��� �ϰ��� �ٴڿ� ������ �����ϰ� �ٴڿ� �����ɽ� �ӵ��� ������ ������ �ǵ����ϴ�.
            if (enteringPressed == true) {
                autoFallDelay = fall;
                enteringPressed = false;
                dualcheck = false;
            }

            try
            {
                // ���� �޴� �κ��Դϴ�.
                Matrix* attackCells = stack->GetCells();
                if (atk2 > 0) {
                    for (int k = 0; k < atk2; k++) {
                        for (int i = 1; i < STACK_HEIGHT; i++) {
                            for (int j = STACK_WIDTH - 1; j >= 0; j--) {
                                stackCells->Set(j, i - 1, stackCells->Get(j, i));
                            }
                        }
                    }
                    for (int i = STACK_HEIGHT - 1; i >= STACK_HEIGHT - atk2; i--) {
                        int hole = rand() % 10;
                        for (int j = 0; j < STACK_WIDTH; j++) {
                            if (j == hole) {
                                stackCells->Set(j, i, 0);
                                continue;
                            }
                            stackCells->Set(j, i, 10);
                        }
                    }
                    atk2 = 0;
                }
            }
            catch (int msg)
            {
                printf("%d\n", msg);
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
            {
                gameOver = true;
                Gover = true;
            }
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
            score2 += static_cast<int>(pow(2, removed) * 100);
            if (autoFallDelay2 > 0.175) {
                    autoFallDelay2 = autoFallDelay2 * 0.98;
            }
            else {
                autoFallDelay2 = 0.175;
            }
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
                        int realx = static_cast<int>(activePiece2->GetPosition().x + j);
                        int realy = static_cast<int>(activePiece2->GetPosition().y + i);
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
                        int realx = static_cast<int>(activePiece2->GetPosition().x + j);
                        int realy = static_cast<int>(activePiece2->GetPosition().y + i);
                        stackCells2->Set(realx, realy, 8);
                    }
                    else if (activePiece2->GetCells()->Get(j, i) > 0)
                    {
                        int realx = static_cast<int>(activePiece2->GetPosition().x + j);
                        int realy = static_cast<int>(activePiece2->GetPosition().y + i);
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

            // ���� �������� ����� Ȯ��, �ӵ��� �ٽ� ������ŵ�ϴ�.
            if (item1_2) {
                autoFallDelay2 = itemfall2;
                item1_2 = false;
                fallcheck2 = false;
            }

            // ��� �ϰ��� �۵��Ǵ� �κ��Դϴ�.
            // ��� �ϰ��� �ٴڿ� ������ �����ϰ� �ٴڿ� �����ɽ� �ӵ��� ������ ������ �ǵ����ϴ�.
            if (enteringPressed2 == true) {
                autoFallDelay2 = fall2;
                enteringPressed2 = false;
                dualcheck2 = false;
            }


            try
            {
                // ���� �޴� �κ��Դϴ�.
                Matrix* attackCells2 = stack2->GetCells();
                if (atk > 0) {
                    for (int k = 0; k < atk; k++) {
                        for (int i = 1; i < STACK_HEIGHT; i++) {
                            for (int j = STACK_WIDTH - 1; j >= 0; j--) {
                                stackCells2->Set(j, i - 1, stackCells2->Get(j, i));
                            }
                        }
                    }
                    for (int i = STACK_HEIGHT - 1; i >= STACK_HEIGHT - atk; i--) {
                        int hole = rand() % 10;
                        for (int j = 0; j < STACK_WIDTH; j++) {
                            if (j == hole) {
                                stackCells2->Set(j, i, 0);
                                continue;
                            }
                            stackCells2->Set(j, i, 10);
                        }
                    }
                    atk = 0;
                }
            }
            catch (int msg)
            {
                printf("%d\n", msg);
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
                Gover = true;
            }
        }
    }

}

HRESULT Engine::Draw()
{
    // �׸��� �׷����� �κ��Դϴ�.
    HRESULT hr;

    // �׸� �׸� �� ����� ������ ���ϴ� �κ��Դϴ�.
    if (m_pRenderTarget != NULL) {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
    }


    // ���� ���� �� ��ҵ�(����, Ȱ�� ��, ��� ��)�� ���������� �׸��� �κ��Դϴ�.
    // Engine ���� �ٷ� �׸��°� �ƴ϶� �� Stack �� Piece���� ����� Draw�� ȣ���� �׸��ϴ�.
        // JPG �̹����� ȭ�鿡 �׸��� �κ� - ������ ��ǥ ������ ����

    DrawImage();

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


    if (wicFactory != NULL) {
        //1p Blind Item
        //1p�� ����ϴ� �Ŷ� 2p�� ���带 �����ϴ�.
        if (item1_3) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\Blind.jpg", 542.0f, 624.0f, 202.0f, -(CELL_SIZE * height2));
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }

        //2p Blind Item
        //2p�� ����ϴ� �Ŷ� 1p�� ���带 �����ϴ�.
        if (item2_3) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\Blind.jpg", 198.0f, 624.0f, 202.0f, -(CELL_SIZE * height));
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
    }

    // �ٷ� �Ʒ��� �۾��� ������ �׸��� �κ��� ȣ����
    // �۾��� �� ������ �׷��ݴϴ�.
    DrawTextAndScore();

    if (m_pRenderTarget != NULL) {
        hr = m_pRenderTarget->EndDraw();
    }

    return S_OK;
}

HRESULT Engine::Draw2()
{
    // �׸��� �׷����� �κ��Դϴ�.
    HRESULT hr;

    // �׸� �׸� �� ����� ������ ���ϴ� �κ��Դϴ�.
    if (m_pRenderTarget != NULL) {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
    }

    //���ȭ��
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\a.png", 0.0f, 0.0f, 900.0f, 800.0f);
    if (FAILED(hr)) {
        // JPG �̹��� �ε� �� �׸��� ����
        return hr;
    }


    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}
HRESULT Engine::DrawImage() 
{
    HRESULT hr;
    if (wicFactory.Get() != NULL) {
        //���ȭ��
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\tetris wall paper.png", 0.0f, 0.0f, 900.0f, 800.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //1p next
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\next.png", 70.0f, 140.0f, 150.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //1p score
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\score.png", 70.0f, 300.0f, 145.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }


        //2p score
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\score.png", 720.0f, 300.0f, 145.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //2p next
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\next.png", 720.0f, 140.0f, 150.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //2p item
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\item.png", 720.0f, 450.0f, 150.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //1p item
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\item.png", 70.0f, 450.0f, 150.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //2p black
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\black.png", 538.0f, 138.0f, 210.0f, 491.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }
        //1p black
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\black.png", 194.0f, 138.0f, 210.0f, 491.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //1p button
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\button.png", 194.0f, 640.0f, 210.0f, 100.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //2p button
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\button.png", 538.0f, 640.0f, 210.0f, 100.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //1p space
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space.png", 194.0f, 750.0f, 210.0f, 30.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //2p space
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space.png", 538.0f, 750.0f, 210.0f, 30.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //logo
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\logo.png", 170.0f, 10.0f, 600.0f, 120.0f);
        if (FAILED(hr)) {
            // JPG �̹��� �ε� �� �׸��� ����
            return hr;
        }

        //1p Ű����
        if (leftPressed == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\left.png", 194, 690, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (rightPressed == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\right.png", 335, 690, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (spacePressed == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\up.png", 264, 640, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (downPressed == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\down.png", 264, 690, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (enteringPressed == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space2.png", 194, 750, 210, 30);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }

        //2p Ű����
        if (leftPressed2 == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\left.png", 538, 691, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (rightPressed2 == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\right.png", 679, 690, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (spacePressed2 == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\up.png", 608, 640, 70, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }

        if (downPressed2 == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\down.png", 608, 690, 68, 50);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
        if (enteringPressed2 == true) {
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space2.png", 538, 750, 210, 30);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
    }
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
    // 
    //�� ���ھ ǥ�õǴ� �κ��Դϴ�.
    if (m_pRenderTarget != NULL) {
        D2D1_RECT_F PScore = D2D1::RectF(centerLeft - 265, padding + 220, centerLeft + 175, padding + 340);
        WCHAR scoreStr[64];
        swprintf_s(scoreStr, L"%d        ", score);
        m_pRenderTarget->DrawText(
            scoreStr,
            7,
            m_pTextFormat,
            PScore,
            m_pWhiteBrush
        );

        //�� ���ھ ǥ�õǴ� �κ��Դϴ�.
        D2D1_RECT_F PScore2 = D2D1::RectF(centerRight, padding + 220, centerRight + 170, padding + 340);
        WCHAR scoreStr2[64];
        swprintf_s(scoreStr2, L"%d        ", score2);
        m_pRenderTarget->DrawText(
            scoreStr2,
            7,
            m_pTextFormat,
            PScore2,
            m_pWhiteBrush
        );

        // �̸� ��� ��ġ�� ���� (���� ��� ��ġ ���������� ����)
        D2D1_RECT_F PName1 = D2D1::RectF(centerRight + 50, padding + 220, centerRight + 270, padding + 340);

        // �̸� ���ڿ� ���� (name1�� ���)
        WCHAR nameStr1[100];
        swprintf_s(nameStr1, L"%s", player1Name.c_str()); // name1�� ����� �̸��� ����Ͽ� ���ڿ� ����

        // �̸� ���
        m_pRenderTarget->DrawText(
            nameStr1,
            wcslen(nameStr1),
            m_pTextFormat,
            PName1,
            m_pWhiteBrush
        );
        // �̸� ��� ��ġ�� ���� (���� ��� ��ġ ���������� ����)
        D2D1_RECT_F PName2 = D2D1::RectF(centerRight - 315, padding + 220, centerRight + 175, padding + 340);

        // �̸� ���ڿ� ���� (name1�� ���)
        WCHAR nameStr2[100];
        swprintf_s(nameStr2, L"%s", player1Name.c_str()); // name1�� ����� �̸��� ����Ͽ� ���ڿ� ����

        // �̸� ���
        m_pRenderTarget->DrawText(
            nameStr2,
            wcslen(nameStr2),
            m_pTextFormat,
            PName2,
            m_pWhiteBrush
        );

        WCHAR ItemStr[64];
        swprintf_s(ItemStr, L"�����۾���");
        D2D1_RECT_F ItemT = D2D1::RectF(centerLeft - 265, padding + 380, centerLeft + 175, padding + 450);
        if (ItemGet == 0) {
            m_pRenderTarget->DrawText(
                ItemStr,
                5,
                m_pTextFormat,
                ItemT,
                m_pWhiteBrush
            );
        }
        else {
            for (int i = 0; i < 6; i++) {
                switch (i) {
                case 0:
                    if (Itemarr[0] > 0) {
                        swprintf_s(ItemStr, L"���ٻ���");
                    }
                    break;
                case 1:
                    if (Itemarr[1] > 0) {
                        swprintf_s(ItemStr, L"��������");
                    }
                    break;
                case 2:
                    if (Itemarr[2] > 0) {
                        swprintf_s(ItemStr, L"����ε�");
                    }
                    break;
                case 3:
                    if (Itemarr[3] > 0) {
                        swprintf_s(ItemStr, L"I �ں�ȯ");
                    }
                    break;
                case 4:
                    if (Itemarr[4] > 0) {
                        swprintf_s(ItemStr, L"��ź��ȯ");
                    }
                    break;
                case 5:
                    if (Itemarr[5] > 0) {
                        swprintf_s(ItemStr, L"���б�");
                    }
                    break;
                default:
                    break;
                }
            }
            m_pRenderTarget->DrawText(
                ItemStr,
                5,
                m_pTextFormat,
                ItemT,
                m_pItemBrush
            );
        }

        WCHAR ItemStr2[64];
        swprintf_s(ItemStr2, L"�����۾���");
        D2D1_RECT_F ItemT2 = D2D1::RectF(centerRight, padding + 380, centerRight + 170, padding + 450);
        if (ItemGet2 == 0) {
            m_pRenderTarget->DrawText(
                ItemStr2,
                5,
                m_pTextFormat,
                ItemT2,
                m_pWhiteBrush
            );

        }
        else {
            for (int i = 0; i < 6; i++) {
                switch (i) {
                case 0:
                    if (Itemarr2[0] > 0) {
                        swprintf_s(ItemStr2, L"���ٻ���");
                    }
                    break;
                case 1:
                    if (Itemarr2[1] > 0) {
                        swprintf_s(ItemStr2, L"��������");
                    }
                    break;
                case 2:
                    if (Itemarr2[2] > 0) {
                        swprintf_s(ItemStr2, L"����ε�");
                    }
                    break;
                case 3:
                    if (Itemarr2[3] > 0) {
                        swprintf_s(ItemStr2, L"I �ں�ȯ");
                    }
                    break;
                case 4:
                    if (Itemarr2[4] > 0) {
                        swprintf_s(ItemStr2, L"��ź��ȯ");
                    }
                    break;
                case 5:
                    if (Itemarr2[5] > 0) {
                        swprintf_s(ItemStr2, L"���б�");
                    }
                    break;
                default:
                    break;
                }
            }
            m_pRenderTarget->DrawText(
                ItemStr2,
                5,
                m_pTextFormat,
                ItemT2,
                m_pItemBrush
            );
        }

        HRESULT hr;
        //���� ������ ��Ÿ���� �κ��Դϴ�.
        if (Gover == true) {
            //gameover
            hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\gameover.png", 325.0f, 230.0f, 300.0f, 300.0f);
            if (FAILED(hr)) {
                // JPG �̹��� �ε� �� �׸��� ����
                return hr;
            }
        }
    }
}

void Engine::setPlayerName(int playerIndex, const std::wstring& name) {
    if (playerIndex == 1) {
        player1Name = name;
    }
    else if (playerIndex == 2) {
        player2Name = name;
    }
}
