#ifndef BASH_SKETCH_H
#define BASH_SKETCH_H

#include <string>
#include <utility>

namespace sketch
{

    class BaseSketch
    {
    public:
        virtual ~BaseSketch() {}

        // Clean the data structure before use
        virtual void clear() = 0;
        // Insert a package to the data structure
        virtual void insert(const std::string &str) = 0;
        // Turn the data into intermediate data for further aggregation
        virtual void work(int n) = 0;
        // Aggregate the data and construct the query data structure
        virtual int merge(int thresh, int opt) = 0;
        // Query the top k flow
        virtual std::pair<std::string, int> query_top(int k);
        // Query the size of the flow
        virtual int query(const std::string &str);
        // Get the name of the sketch
        virtual std::string get_name() = 0;
    };

} // namespace sketch

#endif