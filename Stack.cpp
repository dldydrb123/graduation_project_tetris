#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Engine.h"
#include "item.h"

Stack::Stack() : m_pBlueBrush(NULL), m_pSelectedBrush(NULL), m_pYellowBrush(NULL), m_pBlackBrush(NULL)
{
	cells = new Matrix(STACK_WIDTH, STACK_HEIGHT);
	for (int i = 0; i < 9; i++)
	{
		m_pBrushes[i] = nullptr;
	}
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
		bool check = false;
		bool attack = false;
		bool useitem = false;

		// ��� ĭ�� Ȯ���ϸ鼭 ��ĭ�� �ִٸ� entrieLine�� false�� �ٲ� ���� �Ѿ�ϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			// ��� �ִ� ĭ�� �ִ��� Ȯ��, ����ִٸ� ���� ������ ����
			if (stackCells->Get(j, i) == 0 && check == false)
			{
				entireLine = false;
			}

			// ���� ���� �������� Ȯ���մϴ�.
			// ���ݹ��� ������ ��뿡�� ������ �ȵ˴ϴ�.
			if (stackCells->Get(j, i) == 10) {
				attack = true;
			}

			// ���� ����� �ɽ� �ش� �ٿ� ������ ���� �ִ��� ��Ȯ�ο����� �� ��˻�
			if (j == 9 && check == false && entireLine == true) {
				check = true;
				j = 0;
			}

			// �������� �ִ� Ȯ���� �����Լ��� ������ ����
			if (stackCells->Get(j, i) == 8 && check == true && ItemGet == 0)
			{
				ItemGet = rand() % 6 + 1;

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
		}

		// ���� �������� ���Ǿ��ٸ� entireLine�� true�� �ٲ� ���� ����ϴ�.
		if (i == STACK_HEIGHT - 1 && item1_1 == true)
		{
			entireLine = true;
			useitem = true;
			item1_1 = false;
			Itemarr[0]--;
			ItemGet = 0;
		}

		// ���� ���� ����� ������ ��ĭ�� �����ϴ�.
		// ������ �������� i ���� �ϳ� ���� ������ ���ÿ� �°� �˻��մϴ�.
		if (entireLine)
		{
			removed++;

			if (attack == false && useitem == false) {
				atk++;
			}

			for (int k = i; k > 0; k--)
			{
				for (int j = 0; j < STACK_WIDTH; j++)
				{
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
		bool check = false;
		bool attack = false;
		bool useitem = false;

		// ��� ĭ�� Ȯ���ϸ鼭 ��ĭ�� �ִٸ� entrieLine�� false�� �ٲ� ���� �Ѿ�ϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			
			// ��� �ִ� ĭ�� �ִ��� Ȯ��, ����ִٸ� ���� ������ ����
			if (stackCells->Get(j, i) == 0 && check == false)
			{
				entireLine = false;
			}
			
			// ���� ���� �������� Ȯ���մϴ�.
			// ���ݹ��� ������ ��뿡�� ������ �ȵ˴ϴ�.
			if (stackCells->Get(j, i) == 10) {
				attack = true;
			}
			
			// ���� ����� �ɽ� �ش� �ٿ� ������ ���� �ִ��� ��Ȯ�ο����� �� ��˻�
			if (j == 9 && check == false && entireLine == true) {
				check = true;
				j = 0;
			}

			// �������� �ִ� Ȯ���� �����Լ��� ������ ����
			if (stackCells->Get(j, i) == 8 && check == true && ItemGet2 == 0)
			{
				ItemGet2 = rand() % 6 + 1;

				switch (ItemGet2)
				{
				case 1:
					Itemarr2[0] += 1;
					break;
				case 2:
					Itemarr2[1] += 1;
					break;
				case 3:
					Itemarr2[2] += 1;
					break;
				case 4:
					Itemarr2[3] += 1;
					break;
				case 5:
					Itemarr2[4] += 1;
					break;
				case 6:
					Itemarr2[5] += 1;
					break;
				}
			}
		}

		// ���� �������� ���Ǿ��ٸ� entireLine�� true�� �ٲ� ���� ����ϴ�.
		if (i == STACK_HEIGHT - 1 && item2_1 == true)
		{
			entireLine = true;
			useitem = true;
			item2_1 = false;
			Itemarr2[0]--;
			ItemGet2 = 0;
		}

		// ���� ���� ����� ������ ��ĭ�� �����ϴ�.
		// ������ �������� i ���� �ϳ� ���� ������ ���ÿ� �°� �˻��մϴ�.
		if (entireLine)
		{
			removed++;

			if (attack == false && useitem == false) {
				atk2++;
			}

			for (int k = i; k > 0; k--)
			{
				for (int j = 0; j < STACK_WIDTH; j++)
				{
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

				if (cells->Get(j, i) == 10) {
					m_pSelectedBrush = m_pBrushes[0];
				}

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
	int rightPadding = padding + (STACK_WIDTH + 3) * CELL_SIZE;
	
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

				if (cells->Get(j, i) == 10) {
					m_pSelectedBrush = m_pBrushes[0];
				}

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
