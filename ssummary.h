#ifndef _ssummary_H
#define _ssummary_H

#include <string>

#include "BOBHash32.h"
#include "params.h"

#define len2 9973

/********************Let the Sketch-based method be invertible********************/
class ssummary
{
public:
    int tot;
    int sum[M + 10], K, last[M + 10], Next[M + 10], ID[M + 10], tmp[M + 10];
    int head[N + 10], Left[N + 10], Right[N + 10], num;
    std::string str[M + 10];
    int head2[len2 + 10], Next2[M + 10];
    BOBHash32 *bobhash;

    ssummary(int K);
    ~ssummary();

    void clear();
    int getid();
    int location(const std::string& ST);
    void add2(int x, int y);
    int find(const std::string& s);
    void linkhead(int i, int j);
    void cuthead(int i);
    int getmin();
    void link(int i, int ww);
    void cut(int i);
    void recycling(int i);
};
#endif
