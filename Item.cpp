#include "framework.h"
#include "Item.h"
#include "Matrix.h"
#include "Stack.h"

// 아이템을 획득했는지 체크하는 함수
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

//아이템
//1 - 한줄 삭제
//2 - 세로줄 삭제
//3 - 상대편 가리기
//4 - 블럭 변경
//5 - 떨어트리는 폭탄 생성 (3x3 or 5x5 삭제)
//6 - 속도 증가?

//Piece::Piece() : m_pSelectedBrush(NULL)

Item::Item() 
{
}

Item::~Item() 
{

}

bool Item::RowRemove(Matrix* stackCells) {
	int removed = 0;

	// 스택을 아래서부터 위로 한줄씩 확인합니다.
	for (int i = STACK_HEIGHT - 1; i >= 0; i--)
	{
		bool entireLine = true;
		bool check = false;

		//모든 칸을 확인하면서 빈칸이 있다면 entrieLine을 false로 바꿔 줄을 넘어갑니다.
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

		//// 만약 아이템이 사용되었다면 entireLine을 true로 바꿔 줄을 지웁니다.
		if (i == STACK_HEIGHT - 1)
		{
			entireLine = true;
			Itemarr2[0] -= 1;
		}

		// 꽉찬 줄을 지우고 스택을 한칸씩 내립니다.
		// 스택을 내렸으니 i 값을 하나 내려 내려온 스택에 맞게 검사합니다.
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
