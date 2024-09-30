#pragma once
class Matrix
{
public:
	Matrix(int sizeX, int sizeY);
	~Matrix();

	bool Get(int x, int y);
	void Set(int x, int y, int value);
	int GetXSize();
	int GetYSize();
private:
	int xSize;
	int ySize;

	int** matrix;
};

