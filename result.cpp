#include "result.h"

std::unordered_map<std::string, int> allflowname;
Node *q = nullptr; // results from algorithms
Node *p = nullptr; // true results
std::string ***mergename = nullptr; // key name
int ***mergeresult1 = nullptr;
int ***mergeresult2 = nullptr;
int ***mergeresult3 = nullptr;
int totalnum[10];

int cmp(const Node &i, const Node &j)
{
    return i.y > j.y;
}