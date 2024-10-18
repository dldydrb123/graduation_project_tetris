#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Engine.h"
#include "item.h"
//TODO ���� Ȯ���� ������ ����� �������� �� ���� ä��� �������� �������� �ϱ�
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
	// �׸���� �귯��
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
	// �� �귯�� �ʱ�ȭ
	for (int i = 0; i < 9; i++)
	{
		m_pRenderTarget->CreateSolidColorBrush(colors[i], &m_pBrushes[i]);
	}

}

int Stack::RemoveLines(Matrix* stackCells)
{
	// ���� ���� �����ϴ� �κ��Դϴ�
	int removed = 0;

	// ������ �Ʒ������� ���� ���پ� Ȯ���մϴ�.
	for (int i = STACK_HEIGHT - 1; i >= 0 ; i--)
	{
		bool entireLine = true;

		//��� ĭ�� Ȯ���ϸ鼭 ��ĭ�� �ִٸ� entrieLine�� false�� �ٲ� ���� �Ѿ�ϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (stackCells->Get(j, i) == 0)
			{
				entireLine = false;
			}
		}

		//// ���� �������� ���Ǿ��ٸ� entireLine�� true�� �ٲ� ���� ����ϴ�.
		//if (i == STACK_HEIGHT - 1 && ItemUse == true)
		//{
		//	entireLine = true;
		//	ItemUse = 0;
		//	ItemGet = false;
		//}

		// ���� ���� ����� ������ ��ĭ�� �����ϴ�.
		// ������ �������� i ���� �ϳ� ���� ������ ���ÿ� �°� �˻��մϴ�.
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

// ������ ��� ������ ���� 2p���� ���� �����������ϴ�.
int Stack::RemoveLines2(Matrix* stackCells)
{
	// ���� ���� �����ϴ� �κ��Դϴ�
	int removed = 0;

	// ������ �Ʒ������� ���� ���پ� Ȯ���մϴ�.
	for (int i = STACK_HEIGHT - 1; i >= 0; i--)
	{
		bool entireLine = true;

		//��� ĭ�� Ȯ���ϸ鼭 ��ĭ�� �ִٸ� entrieLine�� false�� �ٲ� ���� �Ѿ�ϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (stackCells->Get(j, i) == 0)
			{
				entireLine = false;
			}
		}

		//// ���� �������� ���Ǿ��ٸ� entireLine�� true�� �ٲ� ���� ����ϴ�.
		//if (i == STACK_HEIGHT - 1 && ItemUse2 == true)
		//{
		//	entireLine = true;
		//	ItemUse2 = 0;
		//	ItemGet2 = false;
		//}

		// ���� ���� ����� ������ ��ĭ�� �����ϴ�.
		// ������ �������� i ���� �ϳ� ���� ������ ���ÿ� �°� �˻��մϴ�.
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
	int shiftX = -46; // x�� �̵� ��
	int shiftY = 30; // y�� �̵� ��
	/*
	// ���� �׸��� �κ��Դϴ�.
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
	// ���� �׸��� �κ��Դϴ�.
	for (int i = 0; i < STACK_HEIGHT; i++)
	{
		bool entireLine = true;

		// ���� ä�����ִ��� �˻��մϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (cells->Get(j, i) == 0)
			{
				entireLine = false;
				break; // ȿ������ ���� �� ���� �߰ߵǸ� �˻� ����
			}
		}

		// ���ο� �ִ� ��� ���� �׸���
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			//��Ʈ���� ��ĭ �׷����� �κ�
			D2D1_RECT_F rectangle4 = D2D1::RectF(
				(padding + RESOLUTION_X / 8) + (j + 1) * CELL_SIZE + 1 + shiftX + CELL_SIZE, padding + i * CELL_SIZE + 1 + shiftY,
				(padding + RESOLUTION_X / 8) + (j + 2) * CELL_SIZE - 1 + shiftX + CELL_SIZE, padding + (i + 1) * CELL_SIZE - 1 + shiftY
			);
			
			if (cells->Get(j, i) > 0)
			{
				m_pSelectedBrush = m_pBrushes[cells->Get(j, i)];

				//���� ä���������� ������ ������ ���������, ���� ����ִٸ� �ʷϻ����� ĥ�մϴ�.
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

// 2p�� ���� ���� �׸��� �κ��Դϴ�.
void Stack::Draw2(ID2D1HwndRenderTarget* m_pRenderTarget)
{
	int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 3;
	int shiftX = -30; // x�� �̵� ��
	int shiftY = 30; // y�� �̵� ��
	int rightPadding = padding + (STACK_WIDTH + 3) * CELL_SIZE; // �� ���� ������
	/*
	// ���� �׸��� �κ��Դϴ�.
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

	//���� �׸��� �κ��Դϴ�.
	for (int i = 0; i < STACK_HEIGHT; i++)
	{
		bool entireLine = true;

		// ���� ä�����ִ��� �˻��մϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (cells->Get(j, i) == 0)
			{
				entireLine = false;
				break; // ȿ������ ���� �� ���� �߰ߵǸ� �˻� ����
			}
		}

		// ���ο� �ִ� ��� ���� �׸���
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			//��Ʈ���� ��ĭ �׷����� �κ�
			D2D1_RECT_F rightrectangle4 = D2D1::RectF(
				(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (j + 1) * CELL_SIZE + 1 + shiftX, padding + i * CELL_SIZE + 1 + shiftY,
				(rightPadding + RESOLUTION_X / 5) + CELL_SIZE + (j + 2) * CELL_SIZE - 1 + shiftX, padding + (i + 1) * CELL_SIZE - 1 + shiftY
			);

			if (cells->Get(j, i) > 0)
			{
				m_pSelectedBrush = m_pBrushes[cells->Get(j, i)];

				//���� ä���������� ������ ������ ��������� ĥ�մϴ�.
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
