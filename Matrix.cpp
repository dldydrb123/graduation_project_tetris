#include "Matrix.h"

Matrix::Matrix(int sizeX, int sizeY)
{
    // 블럭이 담기는 매트릭스입니다.
    xSize = sizeX;
    ySize = sizeY;

    matrix = new int* [ySize];
    for (int i = 0; i < ySize; ++i)
    {
        matrix[i] = new int[xSize];
        for (int j = 0; j < xSize; ++j)
            matrix[i][j] = 0;
    }
}

Matrix::~Matrix()
{
    for (int i = 0; i < ySize; ++i)
    {
        delete[] matrix[i];
    }
    delete[] matrix;
}

int Matrix::Get(int x, int y)
{
    return matrix[y][x];
}

void Matrix::Set(int x, int y, int value)
{
    matrix[y][x] = value;
}
