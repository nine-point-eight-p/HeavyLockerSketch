#ifndef BASH_SKETCH_H
#define BASH_SKETCH_H

#include <string>
#include <utility>

namespace sketch {

class BaseSketch {
public:
    virtual void insert(const std::string& str) = 0;            //insert a package to the data structure
    virtual std::pair<std::string, int> query_top(int k) = 0;   //query the top k flow
    virtual int query(const std::string& str) = 0;                   //query the size of the flow
    virtual void work(int n) = 0;                               //Turn the data into intermediate data for further aggregation
    virtual void clear() = 0;                                   //clean the data structure before use
    virtual int merge(int thresh,int opt) = 0;                  //aggregate the data and construct the query data structure
    virtual std::string get_name() = 0; 
    virtual ~BaseSketch() {}
};

} // namespace sketch


#endif