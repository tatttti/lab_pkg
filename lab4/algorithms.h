#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <QVector>
#include <QString>
#include <chrono>
#include "drawwidget.h"

class AlgorithmResult {
public:
    AlgorithmResult() : m_elapsed(std::chrono::duration<double>::zero()) {}
    void setSteps(const QVector<Step>& s) { m_steps = s; }
    const QVector<Step>& steps() const { return m_steps; }
    void setElapsed(const std::chrono::duration<double> &d) { m_elapsed = d; }
    std::chrono::duration<double> elapsed() const { return m_elapsed; }
private:
    QVector<Step> m_steps;
    std::chrono::duration<double> m_elapsed;
};

class Algorithms {
public:
    static AlgorithmResult stepByStep(const Point &A, const Point &B);
    static AlgorithmResult DDA(const Point &A, const Point &B);
    static AlgorithmResult BresenhamLine(const Point &A, const Point &B);
    static AlgorithmResult BresenhamCircle(const Point &C, const Point &radiusPoint);
    static AlgorithmResult KastlPitvey(const Point &A, const Point &B);
    static AlgorithmResult WuLine(const Point &A, const Point &B);
};

#endif // ALGORITHMS_H

