#include "result.h"

std::unordered_map<std::string, int> allflowname;

Node q[MAXN]; // results from algorithms
Node p[MAXN]; // true results
std::string mergename[10][10][MAX_MEM + 10]; // key name
int mergeresult1[10][10][MAX_MEM + 10];
int mergeresult2[10][10][MAX_MEM + 10];
int mergeresult3[10][10][MAX_MEM + 10];
int totalnum[10];

int cmp(const Node &i, const Node &j)
{
    return i.y > j.y;
}