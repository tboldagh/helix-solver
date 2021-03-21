#pragma once

#include <vector>
#include <string_view>
#include <TTree.h>

#include "Point.h"

class SingleRunMap {
public:
    SingleRunMap(TTree* p_tree, int p_entryNo);

    int getSize() const;
    void addPoint(const Point& p_point);
    void dumpToFile(std::string_view p_fileName) const;

private:
    std::vector<Point> m_points; 
};
