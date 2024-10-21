#pragma once
#include "Matrix.h"
extern int ItemGet;
extern int ItemGet2;

extern int Itemarr[6];
extern int Itemarr2[6];


extern bool item1_1;
extern bool item1_2;
extern bool item1_3;
extern bool item1_4;
extern bool item1_5;
extern bool item1_6;

extern bool item2_1;
extern bool item2_2;
extern bool item2_3;
extern bool item2_4;
extern bool item2_5;
extern bool item2_6;

class Item {
public:
	Item();
	~Item();

	bool RowRemove(Matrix* stackCells);
	void ColRemove();
	void Blind();
	void PieceChan();
	void Bomb();
	void SpeedUp();

private:

};
