#ifndef _RESULTS_H
#define _RESULTS_H

#include <string>
#include <unordered_map>

extern int node_num; // the num of switches in distribution 

/********************Store intermediate data for aggregation********************/
extern std::unordered_map<std::string, int> allflowname;

struct Node { std::string x; int y; };
extern Node *q, *p;
int cmp(const Node& i, const Node& j);

extern std::string ***mergename;
extern int ***mergeresult1;
extern int ***mergeresult2;
extern int ***mergeresult3;
extern int totalnum[10];

#endif