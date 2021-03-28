#include "SingleRunMap.h"
#include <fstream>
#include <algorithm>

SingleRunMap::SingleRunMap(TTree* p_tree, int p_entryNo) {
    std::vector<double>* l_spX = new std::vector<double>();
    std::vector<double>* l_spY = new std::vector<double>();
    std::vector<double>* l_spZ = new std::vector<double>();

    p_tree->SetBranchAddress("spX", &l_spX);
    p_tree->SetBranchAddress("spY", &l_spY);
    p_tree->SetBranchAddress("spZ", &l_spZ);
    p_tree->GetEntry(p_entryNo);

    int l_entrySize = l_spX->size();

    for (int i = 0; i < l_entrySize; ++i) {
        this->addPoint(Point(l_spX->at(i), l_spY->at(i), l_spZ->at(i)));
    }

    delete l_spX;
    delete l_spY;
    delete l_spZ;
}

int SingleRunMap::getSize() const {
    return m_points.size();
}

void SingleRunMap::addPoint(const Point& point) {
    m_points.push_back(point);
}

void SingleRunMap::dumpToFile(std::string_view p_fileName) const {
    std::ofstream l_file;
    l_file.open(p_fileName);
    l_file.precision(64);
    std::for_each(m_points.begin(), m_points.end(),
            [&l_file](const Point& l_point) {
                l_file << l_point.getX() << " " << l_point.getY() << " " << l_point.getZ() << std::endl;
            });
    
    l_file.close();
}
