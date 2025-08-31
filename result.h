#ifndef _RESULTS_H
#define _RESULTS_H

#include <string>
#include <unordered_map>

#include "params.h"

static constexpr int MAXN = 62000005;

extern int node_num; // the num of switches in distribution 

/********************Store intermediate data for aggregation********************/
extern std::unordered_map<std::string, int> allflowname;

struct Node { std::string x; int y; };
extern Node q[MAXN], p[MAXN];
int cmp(const Node& i, const Node& j);

extern std::string mergename[10][10][MAX_MEM + 10];
extern int mergeresult1[10][10][MAX_MEM + 10];
extern int mergeresult2[10][10][MAX_MEM + 10];
extern int mergeresult3[10][10][MAX_MEM + 10];
extern int totalnum[10];

#endif