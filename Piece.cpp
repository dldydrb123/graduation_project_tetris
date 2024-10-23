#include "framework.h"
#include "Point2D.h"
#include "Matrix.h"
#include "Piece.h"
#include "Engine.h"
#include "Item.h"

Piece::Piece() : m_pSelectedBrush(NULL)
{
	//블록의 시작위치
	position.x = STACK_WIDTH / 2 - 2 ;
	position.y = 0;

	waiting = true;

	cells = new Matrix(4, 4);


	// 랜덤으로 블럭을 선택해서 불러옵니다.
	// 블럭 목록은 Piece.h에 있습니다.
	int pieceType = rand() % 7;
	
	// 블럭 체인지 아이템 사용시 블럭을 I자 블럭으로 변경시킵니다.
	if (item1_4 || item2_4) {
		pieceType = 0;
	}
	BrushIndex = pieceType + 1;

	// 폭탄 블럭 아이템 사용시 블럭을 폭탄 블럭으로 변경시킵니다.
	if (item1_5 || item2_5) {
		pieceType = 7;
		BrushIndex = 8;
	}

	//랜덤으로 아이템 블럭인지 정하는 함수, 0~19
	int Item = rand() % 20;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			//1/10 의 확률로 아이템 블럭으로 결정됨
			if (Item < 5 && pieceType != 7) {
				cells->Set(j, i, Itemcells[pieceType][i][j]);
			}
			else {
				cells->Set(j, i, cellsTemplates[pieceType][i][j]);
			}
			
		}
	}
}

Piece::~Piece()
{
	delete cells;
	// 각 브러쉬 COM 객체 해제
	for (int i = 0; i < 9; i++)
	{
		SafeRelease(&m_pBrushes[i]); // 배열의 각 브러쉬 해제
	}
}

void Piece::InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	// 각 브러쉬 초기화
	for (int i = 0; i < 9; i++)
	{
		m_pRenderTarget->CreateSolidColorBrush(colors[i], &m_pBrushes[i]);
	}
	// 무작위로 브러시 선택
	m_pSelectedBrush = m_pBrushes[BrushIndex]; // 선택된 브러쉬를 저장
}

//블럭을 활성화 시키는 부분
void Piece::Activate()
{
	waiting = false;
}

bool Piece::Advance(Matrix* stackCells)
{
	// 블럭이 떨어지는 부분입니다.
	// 스택에서 y값을 하나 떨어트림으로 아래로 한칸씩 내립니다.
	position.y += 1;

	// 만약 스택과 충돌이 일어나면 쌓아야되기 때문에 다시 한칸 위로 올립니다.
	if (StackCollision(stackCells))
	{
		position.y -= 1;
		return true;
	}

	return false;
}

void Piece::GoLeft(Matrix* stackCells)
{
	// 왼쪽방향으로 움직이는 코드입니다.
	int initialPosX = position.x;
	position.x -= 1;

	// 벽이나 스택과 부딪히는지 확인후 부딪히면 움직이지 않습니다.
	if (LeftWallCollision())
	{
		position.x = initialPosX;
		return;
	}

	if (StackCollision(stackCells))
	{
		position.x = initialPosX;
		return;
	}
}

void Piece::GoRight(Matrix* stackCells)
{
	// 오른쪽방향으로 움직이는 코드입니다.
	int initialPosX = position.x;
	position.x += 1;

	// 벽이나 스택과 부딪히는지 확인후 부딪히면 움직이지 않습니다.
	if (RightWallCollision())
	{
		position.x = initialPosX;
		return;
	}

	if (StackCollision(stackCells))
	{
		position.x = initialPosX;
		return;
	}
}

void Piece::Rotate(Matrix* stackCells)
{
	// 돌릴 블럭의 모양을 가져옵니다.
	Matrix* temp = new Matrix(4, 4);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp->Set(j, i, cells->Get(j, i));
		}
	}
	int initialPosX = position.x;

	// 미리 가져온 블럭으로 현재 블럭을 교체합니다.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cells->Set(j, i, temp->Get(i, 3 - j));
		}
	}

	// 왼쪽, 오른쪽 벽과 충돌하는 확인하고 벽과 충돌하면 한칸씩 움직여 벽에 끼는 현상을 방지합니다.
	while(LeftWallCollision())
	{
		position.x += 1;
	};
	while (RightWallCollision())
	{
		position.x -= 1;
	};

	// 스택과 충돌하는지 검사하고, 
	if (StackCollision(stackCells)) {
		// 충돌이 일어나면 다시 원래의 블럭으로 돌립니다.
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				cells->Set(j, i, temp->Get(j, i));
			}
		}
		position.x = initialPosX;
		return;
	}
}

bool Piece::LeftWallCollision()
{
	// 왼쪽 벽과 충돌이 일어나는지 확인하고 충돌이 일어나면 이동을 막습니다.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (cells->Get(j, i) == true)
			{
				int realx = position.x + j;
				if (realx < 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool Piece::RightWallCollision()
{
	// 오른쪽 벽과 충돌이 일어나는지 확인하고 충돌이 일어나면 이동을 막습니다.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (cells->Get(j, i) == true)
			{
				int realx = position.x + j;
				if (realx >= STACK_WIDTH)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool Piece::StackCollision(Matrix* stackCells)
{
	// 밑 바닥이나 스택과 충돌이 일어났는지 확인합니다.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (cells->Get(j, i) > 0)
			{
				int realx = position.x + j;
				int realy = position.y + i;
				// 밑바닥과 충돌이 일어났는지 확인합니다.
				if (realy >= STACK_HEIGHT)
				{
					return true;
				}
				// 다른 스택과 충돌이 일어났는지 확인합니다.
				if (stackCells->Get(realx, realy))
				{
					return true;
				}
			}
		}
	}
	return false;
}

// 블럭을 그리는 부분입니다.
void Piece::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = 30;  // x축 이동 값
	int shiftY = 30;  // y축 이동 값

	int center_x = padding + (position.x + 1) * CELL_SIZE + shiftX-76;
	int center_y = padding + position.y * CELL_SIZE + shiftY;

	// 대기 블럭을 그리는 부분입니다.
	if (waiting)
	{
		center_x = (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 3-220 ;
		center_y = center_y + 50;
	}

	// 활성 블럭을 그리는 부분입니다.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			D2D1_RECT_F rectangle4 = D2D1::RectF(
				center_x + j * CELL_SIZE + 1 + CELL_SIZE + RESOLUTION_X / 8, center_y + i * CELL_SIZE + 1,
				center_x + (j + 1) * CELL_SIZE - 1 + CELL_SIZE + RESOLUTION_X / 8, center_y + (i + 1) * CELL_SIZE - 1
			);

			if (cells->Get(j, i) == 2 || cells->Get(j, i) == 3)
			{
				m_pRenderTarget->FillRectangle(&rectangle4, m_pBrushes[8]);
			}
			else if (cells->Get(j, i) == 1) {
				m_pRenderTarget->FillRectangle(&rectangle4, m_pSelectedBrush);
			}
		}
	}
}

// 2p 블럭을 그리는 부분
void Piece::Draw2(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = 30;  // x축 이동 값
	int shiftY = 30;  // y축 이동 값

	int center_x = padding + ((position.x + STACK_WIDTH + 4) + 1) * CELL_SIZE + shiftX-60;
	int center_y = padding + position.y * CELL_SIZE + shiftY;

	// 대기 블럭을 그리는 부분입니다.
	if (waiting)
	{
		center_x = center_x + 150;
		center_y = center_y + 50;
	}

	// 활성 블럭을 그리는 부분입니다.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			D2D1_RECT_F rectangle4 = D2D1::RectF(
				center_x + j * CELL_SIZE + 1 + RESOLUTION_X / 5, center_y + i * CELL_SIZE + 1,
				center_x + (j + 1) * CELL_SIZE - 1 + RESOLUTION_X / 5, center_y + (i + 1) * CELL_SIZE - 1
			);
			if (cells->Get(j, i) == 2 || cells->Get(j, i) == 3)
			{
				m_pRenderTarget->FillRectangle(&rectangle4, m_pBrushes[8]);
			}
			else if (cells->Get(j, i) == 1)
			{
				m_pRenderTarget->FillRectangle(&rectangle4, m_pSelectedBrush);
			}
		}
	}
}

Point2D Piece::GetPosition()
{
	return position;
}

Matrix* Piece::GetCells()
{
	return cells;
}

// Getter 함수 추가
int Piece :: GetRandomBrushIndex()  {
	return BrushIndex;
}