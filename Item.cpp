#include "framework.h"
#include "Item.h"
#include "Matrix.h"
#include "Stack.h"

// �������� ȹ���ߴ��� üũ�ϴ� �Լ�
int ItemGet = 0;
int ItemGet2 = 0;

int Itemarr[6] = { 0, 0, 0, 0, 0, 0 };
int Itemarr2[6] = { 0, 0, 0, 0, 0, 0 };

bool item1_1 = false;
bool item1_2 = false;
bool item1_3 = false;
bool item1_4 = false;
bool item1_5 = false;
bool item1_6 = false;

bool item2_1 = false;
bool item2_2 = false;
bool item2_3 = false;
bool item2_4 = false;
bool item2_5 = false;
bool item2_6 = false;

//������
//1 - ���� ����
//2 - ������ ����
//3 - ����� ������
//4 - �� ����
//5 - ����Ʈ���� ��ź ���� (3x3 or 5x5 ����)
//6 - �ӵ� ����?

//Piece::Piece() : m_pSelectedBrush(NULL)

Item::Item() 
{
}

Item::~Item() 
{

}

bool Item::RowRemove(Matrix* stackCells) {
	int removed = 0;

	// ������ �Ʒ������� ���� ���پ� Ȯ���մϴ�.
	for (int i = STACK_HEIGHT - 1; i >= 0; i--)
	{
		bool entireLine = true;
		bool check = false;

		//��� ĭ�� Ȯ���ϸ鼭 ��ĭ�� �ִٸ� entrieLine�� false�� �ٲ� ���� �Ѿ�ϴ�.
		for (int j = 0; j < STACK_WIDTH; j++)
		{
			if (stackCells->Get(j, i) == 0 && check == false)
			{
				entireLine = false;
			}

			if (j == 9 && check == false) {
				check = true;
				j = 0;
			}

			if (stackCells->Get(j, i) == 8 && check == true && entireLine == true)
			{
				ItemGet2 = rand() % 6;

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

		//// ���� �������� ���Ǿ��ٸ� entireLine�� true�� �ٲ� ���� ����ϴ�.
		if (i == STACK_HEIGHT - 1)
		{
			entireLine = true;
			Itemarr2[0] -= 1;
		}

		// ���� ���� ����� ������ ��ĭ�� �����ϴ�.
		// ������ �������� i ���� �ϳ� ���� ������ ���ÿ� �°� �˻��մϴ�.
		/*if (entireLine)
		{
			removed++;
			for (int k = i; k > 0; k--)
			{
				for (int j = 0; j < STACK_WIDTH; j++)
				{
					stackCells->Set(j, k, stackCells->Get(j, k - 1));
				}
			}
			i++;
		}*/
	}
	return removed;
}

void Item::ColRemove() {

}

void Item::Blind() {

}

void Item::PieceChan() {

}

void Item::Bomb() {

}

void Item::SpeedUp() {

}
