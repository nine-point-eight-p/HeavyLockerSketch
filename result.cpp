#include "result.h"

std::unordered_map<std::string, int> allflowname;
Node *q = nullptr, *p = nullptr;
std::string ***mergename = nullptr;
int ***mergeresult1 = nullptr;
int ***mergeresult2 = nullptr;
int ***mergeresult3 = nullptr;
int totalnum[10];

int cmp(const Node &i, const Node &j)
{
    return i.y > j.y;
}