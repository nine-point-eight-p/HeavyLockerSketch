
#include <cstring>

#include "ssummary.h"
#include "params.h"

#define rep(i, a, n) for (int i = a; i <= n; i++)

ssummary::ssummary(int K)
    : K(K)
{
    bobhash = new BOBHash32(1000);
}

ssummary::~ssummary()
{
    delete bobhash;
}

void ssummary::clear()
{
    memset(sum, 0, sizeof(sum));
    memset(last, 0, sizeof(Next));
    memset(Next2, 0, sizeof(Next2));
    memset(tmp, 0, sizeof(tmp));
    rep(i, 0, N) head[i] = Left[i] = Right[i] = 0;
    rep(i, 0, len2 + 9) head2[i] = 0;
    tot = 0;
    rep(i, 1, M + 2) ID[i] = i;
    num = M + 2;
    Right[0] = N;
    Left[N] = 0;
}

int ssummary::getid()
{
    int i = ID[num--];
    last[i] = Next[i] = sum[i] = Next2[i] = 0;
    return i;
}

int ssummary::location(const std::string& ST)
{
    return (bobhash->run(ST.c_str(), KEY_LEN)) % len2;
}

void ssummary::add2(int x, int y)
{
    Next2[y] = head2[x];
    head2[x] = y;
}

int ssummary::find(const std::string &s)
{
    for (int i = head2[location(s)]; i; i = Next2[i])
    {
        if (str[i] == s)
            return i;
    }

    return 0;
}

void ssummary::linkhead(int i, int j)
{
    Left[i] = j;
    Right[i] = Right[j];
    Right[j] = i;
    Left[Right[i]] = i;
}

void ssummary::cuthead(int i)
{
    int t1 = Left[i], t2 = Right[i];
    Right[t1] = t2;
    Left[t2] = t1;
}

int ssummary::getmin()
{
    if (tot < K)
        return 0;
    if (Right[0] == N)
        return 1;
    return Right[0];
}

void ssummary::link(int i, int ww)
{
    ++tot;
    bool flag = (head[sum[i]] == 0);
    Next[i] = head[sum[i]];
    if (Next[i])
        last[Next[i]] = i;
    last[i] = 0;
    head[sum[i]] = i;
    if (flag)
    {
        for (int j = sum[i] - 1; j > 0 && j > sum[i] - 100; j--)
            if (head[j])
            {
                linkhead(sum[i], j);
                return;
            }
        linkhead(sum[i], ww);
    }
}

void ssummary::cut(int i)
{
    --tot;
    if (head[sum[i]] == i)
        head[sum[i]] = Next[i];
    if (head[sum[i]] == 0)
        cuthead(sum[i]);
    int t1 = last[i], t2 = Next[i];
    if (t1)
        Next[t1] = t2;
    if (t2)
        last[t2] = t1;
}

void ssummary::recycling(int i)
{
    int w = location(str[i]);
    if (head2[w] == i)
        head2[w] = Next2[i];
    else
    {
        for (int j = head2[w]; j; j = Next2[j])
            if (Next2[j] == i)
            {
                Next2[j] = Next2[i];
                break;
            }
    }
    ID[++num] = i;
}
