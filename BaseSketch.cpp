#include "BaseSketch.h"
#include "result.h"

using namespace sketch;

std::pair<std::string, int> BaseSketch::query_top(int k)
{
    return std::make_pair(q[k].x, q[k].y);
}

int BaseSketch::query(const std::string &str)
{
    auto it = allflowname.find(str);
    return it != allflowname.end() ? it->second : 0;
}