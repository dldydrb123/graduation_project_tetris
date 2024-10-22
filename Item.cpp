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

//아이템
//1 - 한줄 삭제 = RemoveLine에서 확인하고 실행됨
//2 - (스택 변경? -> 구현 오류 봉착) (근데 다른 아이디어는 없음)(하루 박으면 될지도)
//3 - 상대편 가리기 = Engine의 Draw에서 그리고 auto에서 지속시간 체크(3블럭 낙하)
//4 - 다음 블럭 변경 = 새 피스 생성후 waitingPiece와 교체
//5 - 떨어트리는 폭탄 생성 (3x3 or 5x5 삭제) -> 한칸짜리 폭탄 피스 생성, 낙하위치에서 +-해서 제거, 점수는 없음
//											  아이템 획득 가능? 아니면 아이템도 폭발?
//											  아이템 폭발 안하면 폭탄으로 폭탄을 먹고 거기서 폭탄을 먹고 거기서 폭탄....
//6 - 속도 증가? = 이거 버그 많음 ㅈ댐 -> 일단 제거해둠

