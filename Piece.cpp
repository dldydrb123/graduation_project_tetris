#include "framework.h"
#include "Point2D.h"
#include "Matrix.h"
#include "Piece.h"
#include "Engine.h"

Piece::Piece() : m_pSelectedBrush(NULL)
{
	//������ ������ġ
	position.x = STACK_WIDTH / 2 - 2 ;
	position.y = 0;

	waiting = true;

	cells = new Matrix(4, 4);


	// �������� ������ �����ؼ� �ҷ��ɴϴ�.
	// ���� ����� Piece.h�� �ֽ��ϴ�.
	int pieceType = rand() % 7;
	BrushIndex = pieceType + 1;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cells->Set(j, i, cellsTemplates[pieceType][i][j]);
		}
	}
}

Piece::~Piece()
{
	delete cells;
	// �� �귯�� COM ��ü ����
	for (int i = 0; i < 8; i++)
	{
		SafeRelease(&m_pBrushes[i]); // �迭�� �� �귯�� ����
	}
}

void Piece::InitializeD2D(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	// �� �귯�� �ʱ�ȭ
	for (int i = 0; i < 8; i++)
	{
		m_pRenderTarget->CreateSolidColorBrush(colors[i], &m_pBrushes[i]);
	}
	// �������� �귯�� ����
	m_pSelectedBrush = m_pBrushes[BrushIndex]; // ���õ� �귯���� ����
}

//������ Ȱ��ȭ ��Ű�� �κ�
void Piece::Activate()
{
	waiting = false;
}

bool Piece::Advance(Matrix* stackCells)
{
	// ������ �������� �κ��Դϴ�.
	// ���ÿ��� y���� �ϳ� ����Ʈ������ �Ʒ��� ��ĭ�� �����ϴ�.
	position.y += 1;

	// ���� ���ð� �浹�� �Ͼ�� �׾ƾߵǱ� ������ �ٽ� ��ĭ ���� �ø��ϴ�.
	if (StackCollision(stackCells))
	{
		position.y -= 1;
		return true;
	}

	return false;
}

void Piece::GoLeft(Matrix* stackCells)
{
	// ���ʹ������� �����̴� �ڵ��Դϴ�.
	int initialPosX = position.x;
	position.x -= 1;

	// ���̳� ���ð� �ε������� Ȯ���� �ε����� �������� �ʽ��ϴ�.
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
	// �����ʹ������� �����̴� �ڵ��Դϴ�.
	int initialPosX = position.x;
	position.x += 1;

	// ���̳� ���ð� �ε������� Ȯ���� �ε����� �������� �ʽ��ϴ�.
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
	// ���� ������ ����� �����ɴϴ�.
	Matrix* temp = new Matrix(4, 4);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp->Set(j, i, cells->Get(j, i));
		}
	}
	int initialPosX = position.x;

	// �̸� ������ �������� ���� ������ ��ü�մϴ�.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cells->Set(j, i, temp->Get(i, 3 - j));
		}
	}

	// ����, ������ ���� �浹�ϴ� Ȯ���ϰ� ���� �浹�ϸ� ��ĭ�� ������ ���� ���� ������ �����մϴ�.
	while(LeftWallCollision())
	{
		position.x += 1;
	};
	while (RightWallCollision())
	{
		position.x -= 1;
	};

	// ���ð� �浹�ϴ��� �˻��ϰ�, 
	if (StackCollision(stackCells)) {
		// �浹�� �Ͼ�� �ٽ� ������ �������� �����ϴ�.
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
	// ���� ���� �浹�� �Ͼ���� Ȯ���ϰ� �浹�� �Ͼ�� �̵��� �����ϴ�.
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
	// ������ ���� �浹�� �Ͼ���� Ȯ���ϰ� �浹�� �Ͼ�� �̵��� �����ϴ�.
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
	// �� �ٴ��̳� ���ð� �浹�� �Ͼ���� Ȯ���մϴ�.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (cells->Get(j, i) == true)
			{
				int realx = position.x + j;
				int realy = position.y + i;
				// �عٴڰ� �浹�� �Ͼ���� Ȯ���մϴ�.
				if (realy >= STACK_HEIGHT)
				{
					return true;
				}
				// �ٸ� ���ð� �浹�� �Ͼ���� Ȯ���մϴ�.
				if (stackCells->Get(realx, realy))
				{
					return true;
				}
			}
		}
	}
	return false;
}

// ������ �׸��� �κ��Դϴ�.
void Piece::Draw(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = 30;  // x�� �̵� ��
	int shiftY = 30;  // y�� �̵� ��

	int center_x = padding + (position.x + 1) * CELL_SIZE + shiftX;
	int center_y = padding + position.y * CELL_SIZE + shiftY;

	// ��� ������ �׸��� �κ��Դϴ�.
	if (waiting)
	{
		center_x = padding + ((position.x + STACK_WIDTH + 4) + 1) * CELL_SIZE * 2 + shiftX+20;
	}

	// Ȱ�� ������ �׸��� �κ��Դϴ�.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (cells->Get(j, i) == true)
			{
				D2D1_RECT_F rectangle4 = D2D1::RectF(
					center_x + j * CELL_SIZE + 1, center_y + i * CELL_SIZE + 1,
					center_x + (j + 1) * CELL_SIZE - 1, center_y + (i + 1) * CELL_SIZE - 1
				);
				m_pRenderTarget->FillRectangle(&rectangle4, m_pSelectedBrush);
			}
		}
	}
}

// 2p ������ �׸��� �κ�
void Piece::Draw2(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = 30;  // x�� �̵� ��
	int shiftY = 30;  // y�� �̵� ��

	int center_x = padding + ((position.x + STACK_WIDTH + 4) + 1) * CELL_SIZE + shiftX;
	int center_y = padding + position.y * CELL_SIZE + shiftY;

	// ��� ������ �׸��� �κ��Դϴ�.
	if (waiting)
	{
		center_x = padding + ((position.x + STACK_WIDTH + 4) + 1) * CELL_SIZE * 2 + shiftX+20;
		center_y = padding + position.y * CELL_SIZE + 100 + shiftY;
	}

	// Ȱ�� ������ �׸��� �κ��Դϴ�.
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (cells->Get(j, i) == true)
			{
				D2D1_RECT_F rectangle4 = D2D1::RectF(
					center_x + j * CELL_SIZE + 1, center_y + i * CELL_SIZE + 1,
					center_x + (j + 1) * CELL_SIZE - 1, center_y + (i + 1) * CELL_SIZE - 1
				);
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

// Getter �Լ� �߰�
int Piece :: GetRandomBrushIndex()  {
	return BrushIndex;
}