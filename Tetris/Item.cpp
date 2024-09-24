#include "Item.h"
#include <random>

bool ItemGet = false;
bool ItemGet2 = false;
bool SItemGet = false;
bool SItemGet2 = false;
int scorecheck = 0;;
int ItemUse = 0;
int ItemUse2 = 0;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(1, 100);

int random = dis(gen) % 2;