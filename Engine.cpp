#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Piece.h"
#include "Engine.h"
#include "Item.h"
#include <wrl.h> // For Microsoft::WRL::ComPtr
#include <string>

using namespace Microsoft::WRL;

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Windowscodecs.lib")

#define SOLO_PLAY 1

// 전역 변수
ComPtr<ID2D1Factory> d2dFactory;
ComPtr<IWICImagingFactory> wicFactory;
ComPtr<ID2D1RenderTarget> renderTarget;

std::wstring StringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
// Direct2D 텍스트 출력 함수 (파일 내용 출력 예제)
void Engine::DisplayScores(ID2D1HwndRenderTarget* m_pRenderTarget, IDWriteTextFormat* m_pTextFormat, ID2D1SolidColorBrush* m_pWhiteBrush) {
    std::ifstream file("score.txt");
    if (!file.is_open()) {
        // 파일이 없거나 열 수 없으면 종료
        return;
    }

    std::string line;
    float yPos = 255.0f; // 출력 시작 위치 (Y축)
    const float xPosName = 270.0f; // 이름 X 위치
    const float xPosScore = 530.0f; // 점수 X 위치
    const float lineSpacing = 50.0f; // 행 간격

    while (std::getline(file, line, ',')) {
        std::string name;
        int score;

        // 이름을 읽음
        name = line;

        // 점수 부분 읽기
        if (std::getline(file, line, ',')) {
            score = std::stoi(line);
        }
        else {
            break;
        }

        // 이름을 Wide 문자열로 변환하여 출력 준비
        std::wstring nameWStr(name.begin(), name.end());
        D2D1_RECT_F nameRect = D2D1::RectF(xPosName, yPos, xPosName + 75, yPos + 40);
        m_pRenderTarget->DrawText(
            nameWStr.c_str(),
            static_cast<UINT32>(nameWStr.length()),     // Length() 를 UINT32로 변환
            m_pTextFormat,
            nameRect,
            m_pWhiteBrush
        );

        // 점수를 Wide 문자열로 변환하여 출력 준비
        WCHAR scoreStr[64];
        swprintf_s(scoreStr, L"%d", score);
        D2D1_RECT_F scoreRect = D2D1::RectF(xPosScore, yPos, xPosScore + 75, yPos + 40);
        m_pRenderTarget->DrawText(
            scoreStr,
            static_cast<UINT32>(wcslen(scoreStr)),
            m_pTextFormat,
            scoreRect,
            m_pWhiteBrush
        );

        // Y 위치 업데이트 (다음 줄로 이동)
        yPos += lineSpacing;
    }
    file.close();
}

void Engine::SaveHighScore(const std::string& fileName, int score1, const std::wstring& player1Name, int score2, const std::wstring& player2Name) {
    int highestScore;
    std::wstring highestScorer;

    // 두 플레이어 점수 비교
    if (score1 > score2) {
        highestScore = score1;
        highestScorer = player1Name;
    }
    else {
        highestScore = score2;
        highestScorer = player2Name;
    }

    // 파일에 최고 점수와 플레이어 이름을 저장하기
    std::ofstream file(fileName, std::ios::out | std::ios::trunc); // 기존 내용 덮어씌우기
    if (file.is_open()) {
        // UTF-8로 변환하여 저장
        std::string scorerStr(highestScorer.begin(), highestScorer.end());
        file << scorerStr << "," << highestScore << ",";
        file.close();
    }
    else {
        std::cerr << "Unable to open file for writing: " << fileName << std::endl;
    }
}

// DRAW 함수 내에서 실행될 코드
HRESULT Engine::DrawJpgImage(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pWICFactory, const wchar_t* filename, float x, float y, float width, float height)
{
    HRESULT hr = S_OK;
    ComPtr<IWICBitmapDecoder> pDecoder;
    ComPtr<IWICBitmapFrameDecode> pFrame;
    ComPtr<IWICFormatConverter> pConverter;
    ComPtr<ID2D1Bitmap> pBitmap;

    // JPG 이미지를 디코딩
    hr = pWICFactory->CreateDecoderFromFilename(
        filename,                   // 이미지 파일 경로
        nullptr,                    // GUID (자동 선택)
        GENERIC_READ,               // 읽기 전용 모드
        WICDecodeMetadataCacheOnLoad,   // 전체 메타데이터를 로드
        &pDecoder                   // 출력 디코더
    );

    if (FAILED(hr)) {
        return hr;
    }

    // 이미지의 첫 번째 프레임을 가져옵니다.
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        return hr;
    }

    // 포맷 변환기를 생성합니다.
    hr = pWICFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) {
        return hr;
    }

    // 비트맵 포맷을 32bppPBGRA로 변환합니다. (Direct2D와 호환)
    hr = pConverter->Initialize(
        pFrame.Get(),                   // 원본 비트맵 소스
        GUID_WICPixelFormat32bppPBGRA,  // 변환할 포맷
        WICBitmapDitherTypeNone,        // 디더링 없음
        nullptr,                        // 팔레트 없음
        0.f,                            // 알파 임계값 (투명도)
        WICBitmapPaletteTypeCustom      // 사용자 정의 팔레트
    );
    if (FAILED(hr)) {
        return hr;
    }

    // WIC 비트맵을 Direct2D 비트맵으로 변환
    hr = pRenderTarget->CreateBitmapFromWicBitmap(
        pConverter.Get(),
        nullptr,
        &pBitmap
    );
    if (FAILED(hr)) {
        return hr;
    }

    // 비트맵을 렌더 타겟에 그립니다. 고정된 좌표 값을 사용해 그릴 위치를 조정합니다.
    pRenderTarget->DrawBitmap(
        pBitmap.Get(),
        D2D1::RectF(x, y, x + width, y + height)  // 고정된 위치와 크기
    );

    return hr;
}

Engine::Engine() : m_pDirect2dFactory(NULL), m_pRenderTarget(NULL)
{
    // 생성자입니다.
	// Engine.cpp 안에서 사용되는 변수들을 선언하는 공간입니다.
    m_pWhiteBrush = nullptr;
    m_pDWriteFactory = nullptr;
    m_pTextFormat = nullptr;

    srand(static_cast<unsigned int>(time(NULL)));
	
    stack = new Stack();
    stack2 = new Stack();

    activePiece = new Piece();
    activePiece->Activate();
    waitingPiece = new Piece();
    changePiece = new Piece();
    shadowPiece = new Piece();
    shadowPiece->Shadow();

    activePiece2 = new Piece();
    activePiece2->Activate();
    waitingPiece2 = new Piece();
    changePiece2 = new Piece();
    shadowPiece2 = new Piece();
    shadowPiece2->Shadow();

    // autoFall 자동으로 블럭이 떨어지는 속도.
    // 0.7 이 기본값입니다.
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

    // 낙하 확인용 변수
    fcheck = 0;
    scheck = 0;

    // 블라인드 아이템용 변수
    height = 0;
    height2 = 0;

    // 아이템 사용을 확인하는 변수
    blindcheck = 0;
    blindcheck2 = 0;

    // 즉시 하강용 변수
    fall = 0;
    fall2 = 0; 

    // 즉시 하강 아이템용 변수
    itemfall = 0;
    itemfall2 = 0; 
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

    // Direct2D 팩토리 생성
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf());
    if (FAILED(hr)) {
        // 오류 처리
    }

    // WIC 팩토리 생성
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );
    if (FAILED(hr)) {
        // 오류 처리
    }

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
        D2D1::ColorF(D2D1::ColorF::Black),
        &m_pWhiteBrush
    );
}

void Engine::KeyUp(WPARAM wParam)
{
    // 키보드를 눌렀다가 땟을 때 입니다.
    // 입력이 끝나는 타이밍이니까 변수들이 false로 바꾸는걸 확인할수 있습니다.

    // 2p의 이동
    if (wParam == VK_DOWN)
        downPressed2 = false;

    if (wParam == VK_LEFT)
        leftPressed2 = false;

    if (wParam == VK_RIGHT)
        rightPressed2 = false;

    if (wParam == VK_UP)
        spacePressed2 = false;

    // 1p의 이동, 숫자들은 아스키코드로 W,A,S,D 입니다.
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
	// 키를 눌렀을때 입니다.
    // 눌렀을때니까 변수들이 true로 바뀌는걸 확인할수 있습니다.

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

    // 아이템 사용 확인
    // 1p는 키보드 상단 1 ~ 6
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
     
    //2p는 키보드 우측 1 ~ 6
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


    if (!scoreSaved && (gameOver||gameOver2))
    {
        over = true;
        SaveHighScore("score.txt", score, player1Name, score2, player2Name);
        scoreSaved = true;

        return;
    }

    // Stack을 Metrix랑 연결해서 게임판을 만들어 냅니다. (테트리스 쌓이는 부분)
    Matrix* stackCells = stack->GetCells();
    Matrix* changeCells = stack->GetCells();
    Matrix* stackCells2 = stack2->GetCells();
    Matrix* changeCells2 = stack2->GetCells();


    for (int i = STACK_HEIGHT - 1; i >= 0; i--) {
        int k = 0;
        for (int j = 0; j < STACK_WIDTH; j++) {
            if (stackCells->Get(j, i) > 0) {
                break;
            }
            else
            {
                k++;
            }
        }
        if (k == 10) {
            break;
        }
        height = 25 - i;
    }

    for (int i = STACK_HEIGHT - 1; i >= 0; i--) {
        int k = 0;
        for (int j = 0; j < STACK_WIDTH; j++) {
            if (stackCells2->Get(j, i) > 0) {
                break;
            }
            else {
                k++;
            }
        }
        if (k == 10) {
            break;
        }
        height2 = 25 - i;
    }

    // 4번 아이템 사용되는 부분
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
    
    // 5번 아이템 사용되는 부분
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


    // 6번 아이템 사용되는 부분
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
                score += static_cast<int>(pow(2, removed) * 100);
                if (autoFallDelay > 0.175) {
                    autoFallDelay = autoFallDelay * 0.98;
                }
                else {
                    autoFallDelay = 0.175;
                }
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

        // 블럭 즉시 하강을 위해 속도를 증가시킵니다.
        if (enteringPressed) {
            if (dualcheck == false) {
                fall = autoFallDelay;
            }
            autoFallDelay = 0.001;
            dualcheck = true;
        }

        // 2번 아이템을 사용해 상대방의 블럭을 즉시낙하 시킵니다.
        if (item1_2) {
            if (fallcheck2 == false) {
                itemfall2 = autoFallDelay2;
            }
            autoFallDelay2 = 0.001;
            fallcheck2 = true;
            ItemGet = 0;
            Itemarr[1]--;
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
                score2 += static_cast<int>(pow(2, removed) * 100);
                if (autoFallDelay2 > 0.175) {
                    autoFallDelay2 = autoFallDelay2 * 0.98;
                }
                else {
                    autoFallDelay2 = 0.175;
                }
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

        // 블럭 즉시 하강을 위해 속도를 증가시킵니다.
        if (enteringPressed2) {
            if (dualcheck2 == false) {
                fall2 = autoFallDelay2;
            }
            autoFallDelay2 = 0.001;
            dualcheck2 = true;
        }

        // 2번 아이템을 사용해 상대방의 블럭을 즉시낙하 시킵니다.
        if(item2_2){
            if (fallcheck == false) {
                itemfall = autoFallDelay;
            }
            autoFallDelay = 0.001;
            fallcheck = true;
            ItemGet2 = 0;
            Itemarr2[1]--;
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
            score += static_cast<int>(pow(2, removed) * 100);
            if (autoFallDelay > 0.175) {
                autoFallDelay = autoFallDelay * 0.98;
            }
            else {
                autoFallDelay = 0.175;
            }
        }

        // Advance 함수를 호출해 자동으로 블럭을 떨어트리는 부분입니다.
        bool isConflict = activePiece->Advance(stackCells);

        int brushIndex = activePiece->GetRandomBrushIndex();
        // 블럭이 떨어졌을때 그 떨어진 블럭을 스택에 쌓는 부분입니다.
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

            // 블라인드 아이템의 사용을 확인하고 지속시간을 체크합니다.
            if (item2_3) {
                blindcheck++;
                if (blindcheck == 3) {
                    blindcheck = 0;
                    item2_3 = false;
                }
            }

            // 낙하 아이템의 사용을 확인, 속도를 다시 복구시킵니다.
            if (item2_2) {
                autoFallDelay = itemfall;
                item2_2 = false;
                fallcheck = false;
            }

            // 즉시 하강이 작동되는 부분입니다.
            // 즉시 하강시 바닥에 착지를 감지하고 바닥에 착지될시 속도를 원래의 값으로 되돌립니다.
            if (enteringPressed == true) {
                autoFallDelay = fall;
                enteringPressed = false;
                dualcheck = false;
            }

            try
            {
                // 공격 받는 부분입니다.
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
            score2 += static_cast<int>(pow(2, removed) * 100);
            if (autoFallDelay2 > 0.175) {
                    autoFallDelay2 = autoFallDelay2 * 0.98;
            }
            else {
                autoFallDelay2 = 0.175;
            }
        }

        // Advance 함수를 호출해 자동으로 블럭을 떨어트리는 부분입니다.
        bool isConflict = activePiece2->Advance(stackCells2);

        int brushIndex = activePiece2->GetRandomBrushIndex();
        // 블럭이 떨어졌을때 그 떨어진 블럭을 스택에 쌓는 부분입니다.
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

            // 블라인드 아이템의 사용을 확인하고 지속시간을 체크합니다.
            if (item1_3) {
                blindcheck2++;
                if (blindcheck2 == 3) {
                    blindcheck2 = 0;
                    item1_3 = false;
                }
            }

            // 낙하 아이템의 사용을 확인, 속도를 다시 복구시킵니다.
            if (item1_2) {
                autoFallDelay2 = itemfall2;
                item1_2 = false;
                fallcheck2 = false;
            }

            // 즉시 하강이 작동되는 부분입니다.
            // 즉시 하강시 바닥에 착지를 감지하고 바닥에 착지될시 속도를 원래의 값으로 되돌립니다.
            if (enteringPressed2 == true) {
                autoFallDelay2 = fall2;
                enteringPressed2 = false;
                dualcheck2 = false;
            }


            try
            {
                // 공격 받는 부분입니다.
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
        // JPG 이미지를 화면에 그리는 부분 - 고정된 좌표 값으로 전달

    DrawImage();

    stack->Draw(m_pRenderTarget);
    if (gameOver != true) {
        activePiece->Draw(m_pRenderTarget);
        //shadowPiece->Draw(m_pRenderTarget);
    }
    waitingPiece->Draw(m_pRenderTarget);

    stack2->Draw2(m_pRenderTarget);
    if (gameOver2 != true) {
        activePiece2->Draw2(m_pRenderTarget);
        //shadowPiece2->Draw2(m_pRenderTarget);
    }
    waitingPiece2->Draw2(m_pRenderTarget);

    //1p Blind Item
    //1p가 사용하는 거라서 2p의 보드를 가립니다.
    if (item1_3) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\Blind.jpg", 542.0f, 624.0f, 202.0f, -static_cast<float>(CELL_SIZE * height2));
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }

    //2p Blind Item
    //2p가 사용하는 거라서 1p의 보드를 가립니다.
    if (item2_3) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\Blind.jpg", 198.0f, 624.0f, 202.0f, -static_cast<float>(CELL_SIZE * height));
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }

    // 바로 아래의 글씨와 점수를 그리는 부분을 호출해
    // 글씨와 각 점수를 그려줍니다.
    DrawTextAndScore();

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}
//랭킹 출력 고칠 위치
HRESULT Engine::Draw2()
{
    HRESULT hr;

    // Begin drawing
    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

    // 배경화면 그리기
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\a.png", 0.0f, 0.0f, 900.0f, 800.0f);
    if (FAILED(hr)) {
        // 이미지 로드 실패
        return hr;
    }

    // 점수 출력 (DisplayScores 함수 호출)
    DisplayScores(m_pRenderTarget, m_pTextFormat, m_pWhiteBrush);

    // End drawing
    hr = m_pRenderTarget->EndDraw();

    return hr;
}

HRESULT Engine::DrawImage() 
{
    HRESULT hr;

    //배경화면
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\tetris wall paper.png", 0.0f, 0.0f, 900.0f, 800.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //1p next
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\next.png", 70.0f, 140.0f, 150.0f, 120.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //1p score
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\score.png", 70.0f, 300.0f, 145.0f, 120.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }


    //2p score
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\score2.png", 720.0f, 300.0f, 145.0f, 120.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //2p next
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\next.png", 720.0f, 140.0f, 150.0f, 120.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //2p black
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\black.png", 538.0f, 138.0f, 210.0f, 491.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }
    //1p black
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\black.png", 194.0f, 138.0f, 210.0f, 491.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //1p button
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\button.png", 194.0f, 640.0f, 210.0f, 100.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //2p button
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\button.png", 538.0f, 640.0f, 210.0f, 100.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //1p space
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space.png", 194.0f, 750.0f, 210.0f, 30.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //2p space
    hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space.png", 538.0f, 750.0f, 210.0f, 30.0f);
    if (FAILED(hr)) {
        // JPG 이미지 로드 및 그리기 실패
        return hr;
    }

    //1p 키눌림
    if (leftPressed == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\left.png", 194, 690, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (rightPressed == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\right.png", 335, 690, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (spacePressed == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\up.png", 264, 640, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (downPressed == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\down.png", 264, 690, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (enteringPressed == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space2.png", 194, 750, 210, 30);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }

    //2p 키눌림
    if (leftPressed2 == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\left.png", 538, 691, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (rightPressed2 == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\right.png", 679, 690, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (spacePressed2 == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\up.png", 608, 640, 70, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }

    if (downPressed2 == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\down.png", 608, 690, 68, 50);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }
    if (enteringPressed2 == true) {
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\space2.png", 538, 750, 210, 30);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
        }
    }

}

HRESULT Engine::DrawTextAndScore()
{
    // 글씨와 점수를 그리는 부분입니다.

    // 위치 조정용 변수들
    int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
    int centerRight = RESOLUTION_X - (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;
    int centerLeft = (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3;


    // RECT_F를 이용해 글씨 상자를 만들고, 해당 크기만큼 생성된 글씨 상자안에 글씨가 적힙니다.
    // ""안의 내용(Next Piece)들을 그리는데 바로 아래있는 숫자(15)만큼 그립니다.
    // 
    //실 스코어가 표시되는 부분입니다.
    D2D1_RECT_F PScore = D2D1::RectF(centerLeft -265.0f, padding + 220.0f, centerLeft+175.0f, padding + 340.0f);
    WCHAR scoreStr[64];
    swprintf_s(scoreStr, L"%d        ", score);
    m_pRenderTarget->DrawText(
        scoreStr,
        7,
        m_pTextFormat,
        PScore,
        m_pWhiteBrush
    );

    //실 스코어가 표시되는 부분입니다.
    D2D1_RECT_F PScore2 = D2D1::RectF(static_cast<float>(centerRight), padding + 220.0f, centerRight + 170.0f, padding + 340.0f);
    WCHAR scoreStr2[64];
    swprintf_s(scoreStr2, L"%d        ", score2);
    m_pRenderTarget->DrawText(
        scoreStr2,
        7,
        m_pTextFormat,
        PScore2,
        m_pWhiteBrush
    );

    // 이름 출력 위치를 지정 (점수 출력 위치 오른쪽으로 설정)
D2D1_RECT_F PName1 = D2D1::RectF(centerRight +50.0f, padding + 220.0f, centerRight + 270.0f, padding + 340.0f);

// 이름 문자열 생성 (name1을 사용)
WCHAR nameStr1[100];
swprintf_s(nameStr1, L"%s", player1Name.c_str()); // name1에 저장된 이름을 사용하여 문자열 생성

// 이름 출력
m_pRenderTarget->DrawText(
    nameStr1,
    static_cast<UINT32>(wcslen(nameStr1)),
    m_pTextFormat,
    PName1,
    m_pWhiteBrush
);
// 이름 출력 위치를 지정 (점수 출력 위치 오른쪽으로 설정)
D2D1_RECT_F PName2 = D2D1::RectF(centerRight - 315.0f, padding + 220.0f, centerRight + 175.0f, padding + 340.0f);

// 이름 문자열 생성 (name1을 사용)
WCHAR nameStr2[100];
swprintf_s(nameStr2, L"%s", player1Name.c_str()); // name1에 저장된 이름을 사용하여 문자열 생성

// 이름 출력
m_pRenderTarget->DrawText(
    nameStr2,
    static_cast<UINT32>(wcslen(nameStr2)),
    m_pTextFormat,
    PName2,
    m_pWhiteBrush
);
    WCHAR ItemStr[64];
    swprintf_s(ItemStr, L"아이템없음");
    
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0:
            if (Itemarr[0] > 0) {
                swprintf_s(ItemStr, L"한줄삭제");
            }
            break;
        case 1:
            if (Itemarr[1] > 0) {
                swprintf_s(ItemStr, L"강제낙하");
            }
            break;
        case 2:
            if (Itemarr[2] > 0) {
                swprintf_s(ItemStr, L"블라인드");
            }
            break;
        case 3:
            if (Itemarr[3] > 0) {
                swprintf_s(ItemStr, L"I 자변환");
            }
            break;
        case 4:
            if (Itemarr[4] > 0) {
                swprintf_s(ItemStr, L"폭탄변환");
            }
            break;
        case 5:
            if (Itemarr[5] > 0) {
                swprintf_s(ItemStr, L"상대밀기");
            }
            break;
        default:
            break;
        }
    }

    D2D1_RECT_F ItemT = D2D1::RectF(centerLeft - 265.0f, padding + 380.0f, centerLeft + 175.0f, padding + 450.0f);
    m_pRenderTarget->DrawText(
        ItemStr,
        5,
        m_pTextFormat,
        ItemT,
        m_pWhiteBrush
    );

    WCHAR ItemStr2[64];
    swprintf_s(ItemStr2, L"아이템없음");
    for (int i = 0; i < 6; i++) {
        switch (i) {
        case 0:
            if (Itemarr2[0] > 0) {
                swprintf_s(ItemStr2, L"한줄삭제");
            }
            break;
        case 1:
            if (Itemarr2[1] > 0) {
                swprintf_s(ItemStr2, L"강제낙하");
            }
            break;
        case 2:
            if (Itemarr2[2] > 0) {
                swprintf_s(ItemStr2, L"블라인드");
            }
            break;
        case 3:
            if (Itemarr2[3] > 0) {
                swprintf_s(ItemStr2, L"I 자변환");
            }
            break;
        case 4:
            if (Itemarr2[4] > 0) {
                swprintf_s(ItemStr2, L"폭탄변환");
            }
            break;
        case 5:
            if (Itemarr2[5] > 0) {
                swprintf_s(ItemStr2, L"상대밀기");
            }
            break;
        default:
            break;
        }
    }

    D2D1_RECT_F ItemT2 = D2D1::RectF(static_cast<float>(centerRight), padding + 380.0f, centerRight + 170.0f, padding + 450.0f);
    m_pRenderTarget->DrawText(
        ItemStr2,
        5,
        m_pTextFormat,
        ItemT2,
        m_pWhiteBrush
    );

    HRESULT hr;
    //게임 오버시 나타나는 부분입니다.
    if (over == true) {
        //gameover
        hr = DrawJpgImage(m_pRenderTarget, wicFactory.Get(), L"image\\gameover.png", 325.0f, 230.0f, 300.0f, 300.0f);
        if (FAILED(hr)) {
            // JPG 이미지 로드 및 그리기 실패
            return hr;
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

