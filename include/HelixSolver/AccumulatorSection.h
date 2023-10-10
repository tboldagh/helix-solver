#pragma once
#include "HelixSolver/Constants.h"
namespace HelixSolver {

class AccumulatorSection {
public:
  AccumulatorSection() = default;
  AccumulatorSection(double xw, double yw, double xBegin, double yBegin,
                     int div)
      : xSize(xw), ySize(yw), xBegin(xBegin), yBegin(yBegin),
        divisionLevel(div) {
        }

  double xSize;
  double ySize;
  double xBegin;
  double yBegin;
  uint32_t divisionLevel = 0; // number of divisions needed from the original acc
  short indices[MAX_COUNT_PER_SECTION];
  uint8_t counts = 0;
  int8_t OUT_OF_RANGE_COUNTS = 111;
  inline bool canUseIndices() const { return counts != 0; }

  AccumulatorSection bottomLeft(float xFraction = 0.5,
                                float yFraction = 0.5) const {
                                    return AccumulatorSection(
                                        xSize*xFraction,
                                        ySize*yFraction,
                                        xBegin,
                                        yBegin,
                                        divisionLevel+1);
                                }
  AccumulatorSection topLeft(float xFraction = 0.5,
                             float yFraction = 0.5) const {
                                    return AccumulatorSection(
                                        xSize*xFraction,
                                        ySize*yFraction,
                                        xBegin,
                                        yBegin+ySize-ySize*yFraction,
                                        divisionLevel+1);
                             }
  AccumulatorSection topRight(float xFraction = 0.5,
                              float yFraction = 0.5) const {
                                    return AccumulatorSection(
                                        xSize*xFraction,
                                        ySize*yFraction,
                                        xBegin+xSize-xSize*xFraction,
                                        yBegin+ySize-ySize*yFraction,
                                        divisionLevel+1);

                              }
  AccumulatorSection bottomRight(float xFraction = 0.5,
                                 float yFraction = 0.5) const {
                                    return AccumulatorSection(
                                        xSize*xFraction,
                                        ySize*yFraction,
                                        xBegin+xSize-xSize*xFraction,
                                        yBegin,
                                        divisionLevel+1);
                              }
  AccumulatorSection bottom(float yFraction = 0.5) const {
    return bottomLeft(1.0, yFraction);
  }
  AccumulatorSection top(float yFraction = 0.5) const {
    return topLeft(1.0, yFraction);
  }
  AccumulatorSection left(float xFraction = 0.5) const {
    return bottomLeft(xFraction, 1.0);
  }
  AccumulatorSection right(float xFraction = 0.5) const {
    return bottomRight(xFraction, 1.0);
  }

  bool isLineInside( float a, float b ) const {
    const float yB = fma(a, xBegin, b);
    const float yE = fma(a, (xBegin+xSize), b);
    return yB < yBegin+ySize && yE > yBegin;
  }

  // counter clock wise distance from upper left corner
  // a and b are line parameters y = ax + b
  float distCC(float a, float b) const {
    const float y = fma(a, (xBegin + xSize), b);
    const float yEnd = yBegin+ySize;
    return y <= yEnd ? (xSize + (yEnd - y)) : ((yEnd - b)/a - xBegin);
  }
  // anti-counter clock wise distance from upper left corner
  float distACC(float a, float b) const {
    const float y = fma(a, xBegin, b);
    return y <= yBegin ? (ySize + (yBegin-b)/a - xBegin) : (yBegin+ySize - y );
  }


};

} // namespace HelixSolver