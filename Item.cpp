#include "framework.h"
#include "Item.h"
#include "Matrix.h"
#include "Stack.h"

// �������� ȹ���ߴ��� üũ�ϴ� �Լ�
int ItemGet = 0;
int ItemGet2 = 0;

// �� ������ ������ ����Ǵ� ���
int Itemarr[6] = { 0, 0, 0, 0, 0, 0 };
int Itemarr2[6] = { 0, 0, 0, 0, 0, 0 };

// �� �������� ����ϴ��� Ȯ���ϴ� �Լ�
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

bool Gover = false;
bool reset = false;

int atk = 0;
int atk2 = 0;

//������
 
// ������ ���� - ���� ���� X
//             - �ѹ��� ���� �Ѱ���, ������ ��ø �ȵ�, ������ �Ҹ� ��Ű�Բ� 
 
//���� ����
//
// �������� �̸� �����ֱ�
// �帴�� ������ ���������� �̸� �����ֱ�
//
// ���� ���� ��� ����
// ������ �ӵ������ؼ� �߰��� Ű �ԷµǴϱ� �װ� Ű�Է� �� �ð��� ���� �ٷ� �����ϰ� �ϱ�