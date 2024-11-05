#include "framework.h"
#include "Item.h"
#include "Matrix.h"
#include "Stack.h"

// 아이템을 획득했는지 체크하는 함수
int ItemGet = 0;
int ItemGet2 = 0;

// 각 아이템 갯수가 저장되는 어레이
int Itemarr[6] = { 0, 0, 0, 0, 0, 0 };
int Itemarr2[6] = { 0, 0, 0, 0, 0, 0 };

// 각 아이템이 사용하는지 확인하는 함수
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

//아이템
 
// 아이템 보관 - 갯수 누적 X
//             - 한번에 오직 한개만, 아이템 중첩 안됨, 빠르게 소모 시키게끔 
 
//개인 구현
//
// 낙하지점 미리 보여주기
// 흐릿한 블럭으로 낙하지점을 미리 보여주기
//
// 엔터 낙하 즉시 시전
// 지금은 속도증가해서 중간에 키 입력되니까 그거 키입력 될 시간도 없이 바로 낙하하게 하기