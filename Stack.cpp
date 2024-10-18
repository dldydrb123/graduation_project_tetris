#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Engine.h"
#include "item.h"
//TODO 일정 확률로 아이템 블록이 떨어져서 그 줄을 채우면 아이템을 가져가게 하기
Stack::Stack() : m_pBlueBrush(NULL), m_pSelectedBrush(NULL), m_pYellowBrush(NULL), m_pBlackBrush(NULL)
{
	cells = new Matrix(STACK_WIDTH, STACK_HEIGHT);
}

Stack::~Stack()
{
	delete cells;
	SafeRelease(&m_pBlueBrush);
	SafeRelease(&m_pSelectedBrush);
	SafeRelease(&m_pYellowBrush);
	SafeRelease(&m_pYellowBrush);
}

void Stack::InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	// 그리기용 브러쉬
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Blue),
		&m_pBlueBrush
	);
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Yellow),
		&m_pYellowBrush
	);
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&m_pBlackBrush
	);
	// 각 브러쉬 초기화
	for (int i = 0; i < 9; i++)
	{
		m_pRenderTarget->CreateSolidColorBrush(colors[i], &m_pBrushes[i]);
	}

}

int Stack::RemoveLines(Matrix* stackCells)
{
	// 꽉찬 열을 삭제하는 부분입니다
	int removed = 0;

	// 스택을 아래서부터 위로 한줄씩 확인합니다.
	for (int i = STACK_HEIGHT - 1; i >= 0 ; i--)
	{
		bool entireLine = true;

		//모든 칸을 확인하면서 빈칸이 있다면 entrieLine을 false로 바꿔 줄을 넘어갑니다.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (stackCells->Get(j, i) == 0)
			{
				entireLine = false;
			}
		}

		//// 만약 아이템이 사용되었다면 entireLine을 true로 바꿔 줄을 지웁니다.
		//if (i == STACK_HEIGHT - 1 && ItemUse == true)
		//{
		//	entireLine = true;
		//	ItemUse = 0;
		//	ItemGet = false;
		//}

		// 꽉찬 줄을 지우고 스택을 한칸씩 내립니다.
		// 스택을 내렸으니 i 값을 하나 내려 내려온 스택에 맞게 검사합니다.
		if (entireLine)
		{
			removed++;
			for (int k = i; k > 0; k--)
			{
				for (int j = 0; j < STACK_WIDTH; j++)
				{
					if (stackCells->Get(j, k) == 8)
					{
						ItemGet = rand() % 7;
					}
					stackCells->Set(j, k, stackCells->Get(j, k - 1));
				}
			}
			i++;
		}
	}
	return removed;
}

// 아이템 사용 구분을 위해 2p용이 따로 나누어졌습니다.
int Stack::RemoveLines2(Matrix* stackCells)
{
	// 꽉찬 열을 삭제하는 부분입니다
	int removed = 0;

	// 스택을 아래서부터 위로 한줄씩 확인합니다.
	for (int i = STACK_HEIGHT - 1; i >= 0; i--)
	{
		bool entireLine = true;

		//모든 칸을 확인하면서 빈칸이 있다면 entrieLine을 false로 바꿔 줄을 넘어갑니다.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (stackCells->Get(j, i) == 0)
			{
				entireLine = false;
			}
		}

		//// 만약 아이템이 사용되었다면 entireLine을 true로 바꿔 줄을 지웁니다.
		//if (i == STACK_HEIGHT - 1 && ItemUse2 == true)
		//{
		//	entireLine = true;
		//	ItemUse2 = 0;
		//	ItemGet2 = false;
		//}

		// 꽉찬 줄을 지우고 스택을 한칸씩 내립니다.
		// 스택을 내렸으니 i 값을 하나 내려 내려온 스택에 맞게 검사합니다.
		if (entireLine)
		{
			removed++;
			for (int k = i; k > 0; k--)
			{
				for (int j = 0; j < STACK_WIDTH; j++)
				{
					if (stackCells->Get(j, k) == 8)
					{
						ItemGet2 = rand() % 6;

						switch (ItemGet2)
						{
						case 1:
							Item2[0] += 1;
							break;
						case 2:
							Item2[1] += 1;
							break;
						case 3:
							Item2[2] += 1;
							break;
						case 4:
							Item2[3] += 1;
							break;
						case 5:
							Item2[4] += 1;
							break;
						case 6:
							Item2[5] += 1;
							break;
						}
					}
					stackCells->Set(j, k, stackCells->Get(j, k - 1));
				}
			}
			i++;
		}
	}
	return removed;
}

void Stack::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = -46; // x축 이동 값
	int shiftY = 30; // y축 이동 값
	/*
	// 벽을 그리는 부분입니다.
	D2D1_RECT_F rectangle1 = D2D1::RectF(
		(padding + RESOLUTION_X / 8) + shiftX + CELL_SIZE, padding + shiftY,
		(padding + RESOLUTION_X / 8) + CELL_SIZE + CELL_SIZE + shiftX, padding + (STACK_HEIGHT + 1) * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rectangle1, m_pBlueBrush);

	D2D1_RECT_F rectangle2 = D2D1::RectF(
		(padding + RESOLUTION_X / 8) + shiftX + CELL_SIZE, padding + STACK_HEIGHT * CELL_SIZE + shiftY,
		(padding + RESOLUTION_X / 8) + (STACK_WIDTH + 2) * CELL_SIZE + CELL_SIZE + shiftX, padding + (STACK_HEIGHT + 1) * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rectangle2, m_pBlueBrush);

	D2D1_RECT_F rectangle3 = D2D1::RectF(
		(padding + RESOLUTION_X / 8) + (STACK_WIDTH + 1) * CELL_SIZE + shiftX + CELL_SIZE, padding + shiftY,
		(padding + RESOLUTION_X / 8) + (STACK_WIDTH + 2) * CELL_SIZE + CELL_SIZE + shiftX, padding + (STACK_HEIGHT + 1) * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rectangle3, m_pBlueBrush);

	D2D1_RECT_F rectangle5 = D2D1::RectF(
		(padding + RESOLUTION_X / 8) + 2 * CELL_SIZE + shiftX, padding + shiftY,
		(padding + RESOLUTION_X / 8) + (STACK_WIDTH + 2) * CELL_SIZE + shiftX, padding + STACK_HEIGHT * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rectangle5, m_pBlackBrush);
	*/
	// 블럭을 그리는 부분입니다.
	for (int i = 0; i < STACK_HEIGHT; i++)
	{
		bool entireLine = true;

		// 열이 채워져있는지 검사합니다.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (cells->Get(j, i) == 0)
			{
				entireLine = false;
				break; // 효율성을 위해 빈 셀이 발견되면 검사 중지
			}
		}

		// 라인에 있는 모든 셀을 그리기
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			//테트리스 한칸 그려지는 부분
			D2D1_RECT_F rectangle4 = D2D1::RectF(
				(padding + RESOLUTION_X / 8) + (j + 1) * CELL_SIZE + 1 + shiftX + CELL_SIZE, padding + i * CELL_SIZE + 1 + shiftY,
				(padding + RESOLUTION_X / 8) + (j + 2) * CELL_SIZE - 1 + shiftX + CELL_SIZE, padding + (i + 1) * CELL_SIZE - 1 + shiftY
			);
			
			if (cells->Get(j, i) > 0)
			{
				m_pSelectedBrush = m_pBrushes[cells->Get(j, i)];

				//열이 채워져있으면 없어질 스택을 노란색으로, 열이 비어있다면 초록색으로 칠합니다.
				if (entireLine)
				{
					m_pRenderTarget->FillRectangle(&rectangle4, m_pYellowBrush);
				}
				else
				{
					m_pRenderTarget->FillRectangle(&rectangle4, m_pSelectedBrush);
				}
			}
		}
	}
}

// 2p용 벽과 블럭을 그리는 부분입니다.
void Stack::Draw2(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = -30; // x축 이동 값
	int shiftY = 30; // y축 이동 값
	int rightPadding = padding + (STACK_WIDTH + 3) * CELL_SIZE; // 새 벽의 시작점
	/*
	// 벽을 그리는 부분입니다.
	D2D1_RECT_F rightRectangle1 = D2D1::RectF(
		(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + shiftX, padding + shiftY,
		(rightPadding + RESOLUTION_X / 5) + CELL_SIZE * 2 + shiftX, padding + (STACK_HEIGHT + 1) * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rightRectangle1, m_pBlueBrush);

	D2D1_RECT_F rightRectangle2 = D2D1::RectF(
		(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + shiftX, padding + STACK_HEIGHT * CELL_SIZE + shiftY,
		(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (STACK_WIDTH + 2) * CELL_SIZE + shiftX, padding + (STACK_HEIGHT + 1) * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rightRectangle2, m_pBlueBrush);

	D2D1_RECT_F rightRectangle3 = D2D1::RectF(
		(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (STACK_WIDTH + 1) * CELL_SIZE + shiftX, padding + shiftY,
		(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (STACK_WIDTH + 2) * CELL_SIZE + shiftX, padding + (STACK_HEIGHT + 1) * CELL_SIZE + shiftY
	);
	m_pRenderTarget->FillRectangle(&rightRectangle3, m_pBlueBrush);

	D2D1_RECT_F rectangle5 = D2D1::RectF(
		(rightPadding + RESOLUTION_X / 5) + 2 * CELL_SIZE + shiftX, padding + shiftY,
		(rightPadding + RESOLUTION_X / 5) + (STACK_WIDTH + 2) * CELL_SIZE + shiftX, padding + STACK_HEIGHT * CELL_SIZE + shiftY
	); 
	m_pRenderTarget->FillRectangle(&rectangle5, m_pBlackBrush);
	*/

	//블럭을 그리는 부분입니다.
	for (int i = 0; i < STACK_HEIGHT; i++)
	{
		bool entireLine = true;

		// 열이 채워져있는지 검사합니다.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (cells->Get(j, i) == 0)
			{
				entireLine = false;
				break; // 효율성을 위해 빈 셀이 발견되면 검사 중지
			}
		}

		// 라인에 있는 모든 셀을 그리기
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			//테트리스 한칸 그려지는 부분
			D2D1_RECT_F rightrectangle4 = D2D1::RectF(
				(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (j + 1) * CELL_SIZE + 1 + shiftX, padding + i * CELL_SIZE + 1 + shiftY,
				(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (j + 2) * CELL_SIZE - 1 + shiftX, padding + (i + 1) * CELL_SIZE - 1 + shiftY
			);

			if (cells->Get(j, i) > 0)
			{
				m_pSelectedBrush = m_pBrushes[cells->Get(j, i)];

				//열이 채워져있으면 없어질 스택을 노란색으로 칠합니다.
				if (entireLine)
				{
					m_pRenderTarget->FillRectangle(&rightrectangle4, m_pYellowBrush);
				}
				else
				{
					m_pRenderTarget->FillRectangle(&rightrectangle4, m_pSelectedBrush);
				}
			}
		}
	}
}
Matrix* Stack::GetCells()
{
	return cells;
}
