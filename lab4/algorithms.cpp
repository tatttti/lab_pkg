#include "algorithms.h"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <QSet>
#include <QPair>

using namespace std::chrono;

static Step makeStep(int x, int y, const QString &note = QString(), double intensity = 1.0) {
    Step s;
    s.setPoint(Point(x,y));
    s.setNote(note);
    s.setIntensity(intensity);
    return s;
}

static void swapInts(int &a, int &b) { int t=a;a=b;b=t; }

static void appendIfNew(QVector<Step> &container, const Step &s) {
    if (container.isEmpty()) { container.append(s); return; }
    Point prev = container.last().point();
    Point cur = s.point();
    if (prev.x() == cur.x() && prev.y() == cur.y()) return;
    container.append(s);
}

AlgorithmResult Algorithms::stepByStep(const Point &A, const Point &B) {
    AlgorithmResult res;
    auto t0 = high_resolution_clock::now();
    int x0 = A.x(), y0 = A.y(), x1 = B.x(), y1 = B.y();
    int dx = x1 - x0, dy = y1 - y0;
    QVector<Step> steps;
    if (dx == 0) {
        int sy = (dy >= 0) ? 1 : -1;
        for (int y = y0; ; y += sy) {
            appendIfNew(steps, makeStep(x0, y, "v", 1.0));
            if (y == y1) break;
        }
    } else {
        double m = double(dy)/double(dx);
        int sx = (dx >= 0) ? 1 : -1;
        int x = x0;
        while (true) {
            double yf = y0 + m*(x - x0);
            appendIfNew(steps, makeStep(x, int(std::round(yf)), QString::number(yf), 1.0));
            if (x == x1) break;
            x += sx;
        }
    }
    res.setSteps(steps);
    res.setElapsed(high_resolution_clock::now() - t0);
    return res;
}

AlgorithmResult Algorithms::DDA(const Point &A, const Point &B) {
    AlgorithmResult res;
    auto t0 = high_resolution_clock::now();
    int x0 = A.x(), y0 = A.y(), x1 = B.x(), y1 = B.y();
    int dx = x1 - x0, dy = y1 - y0;
    QVector<Step> steps;
    int L = std::max(std::abs(dx), std::abs(dy));
    if (L == 0) {
        steps.append(makeStep(x0,y0,"degenerate",1.0));
        res.setSteps(steps);
        res.setElapsed(high_resolution_clock::now() - t0);
        return res;
    }
    double sx = double(dx)/L;
    double sy = double(dy)/L;
    double x = x0, y = y0;
    for (int i = 0; i <= L; ++i) {
        appendIfNew(steps, makeStep(int(std::round(x)), int(std::round(y)), QString("i=%1").arg(i), 1.0));
        x += sx; y += sy;
    }
    res.setSteps(steps);
    res.setElapsed(high_resolution_clock::now() - t0);
    return res;
}

AlgorithmResult Algorithms::BresenhamLine(const Point &A, const Point &B) {
    AlgorithmResult res;
    auto t0 = high_resolution_clock::now();
    int x0 = A.x(), y0 = A.y(), x1 = B.x(), y1 = B.y();
    int dx = std::abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy; // err = dx - |dy|
    QVector<Step> steps;
    int x = x0, y = y0;
    while (true) {
        appendIfNew(steps, makeStep(x,y, QString("err=%1").arg(err), 1.0));
        if (x == x1 && y == y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x += sx; }
        if (e2 <= dx) { err += dx; y += sy; }
    }
    res.setSteps(steps);
    res.setElapsed(high_resolution_clock::now() - t0);
    return res;
}

AlgorithmResult Algorithms::BresenhamCircle(const Point &C, const Point &radiusPoint) {
    AlgorithmResult res;
    auto t0 = high_resolution_clock::now();
    int R = std::abs(radiusPoint.x());
    QVector<Step> steps;
    if (R <= 0) {
        steps.append(makeStep(C.x(), C.y(), "radius zero", 1.0));
        res.setSteps(steps);
        res.setElapsed(high_resolution_clock::now() - t0);
        return res;
    }
    QSet<QPair<int,int>> used;
    auto addUnique = [&](int xx, int yy){
        QPair<int,int> key(xx,yy);
        if (!used.contains(key)) {
            used.insert(key);
            appendIfNew(steps, makeStep(xx, yy, QString(), 1.0));
        }
    };

    int x = 0, y = R;
    int d = 3 - 2*R;
    while (x <= y) {
        addUnique(C.x() + x, C.y() + y);
        addUnique(C.x() - x, C.y() + y);
        addUnique(C.x() + x, C.y() - y);
        addUnique(C.x() - x, C.y() - y);
        addUnique(C.x() + y, C.y() + x);
        addUnique(C.x() - y, C.y() + x);
        addUnique(C.x() + y, C.y() - x);
        addUnique(C.x() - y, C.y() - x);
        if (d <= 0) d = d + 4*x + 6;
        else { d = d + 4*(x - y) + 10; y--; }
        x++;
    }
    res.setSteps(steps);
    res.setElapsed(high_resolution_clock::now() - t0);
    return res;
}

AlgorithmResult Algorithms::KastlPitvey(const Point &Aorig, const Point &Borig) {
    AlgorithmResult res;
    auto t0 = high_resolution_clock::now();

    int x0 = Aorig.x(), y0 = Aorig.y(), x1 = Borig.x(), y1 = Borig.y();

    if (x0 == x1 && y0 == y1) {
        QVector<Step> steps;
        steps.append(makeStep(x0, y0, "degenerate", 1.0));
        res.setSteps(steps);
        res.setElapsed(high_resolution_clock::now() - t0);
        return res;
    }

    int ax = x0, ay = y0, bx = x1, by = y1;
    if (bx < ax) { swapInts(ax, bx); swapInts(ay, by); }

    int dx = bx - ax;
    int dy = by - ay;

    bool swapped_xy = false;
    bool reflect_x = false;
    bool reflect_y = false;

    if (dy < 0) { ay = -ay; by = -by; dy = -dy; reflect_y = true; }
    if (dx < 0) { ax = -ax; bx = -bx; dx = -dx; reflect_x = true; }
    if (dy > dx) { swapInts(ax, ay); swapInts(bx, by); swapInts(dx, dy); swapped_xy = true; }

    QVector<int> lowerY; lowerY.reserve(dx+1);
    QVector<int> upperY; upperY.reserve(dx+1);
    for (int xi = 0; xi <= dx; ++xi) {
        double xr = ax + xi;
        double yf = ay + (dx == 0 ? 0.0 : double(dy) * double(xr - ax) / double(dx));
        int lf = int(std::floor(yf + 1e-9));
        int uf = int(std::ceil(yf - 1e-9));
        lowerY.append(lf);
        upperY.append(uf);
    }

    QString rote;
    int ycur = lowerY[0];
    for (int i = 0; i < dx; ++i) {
        bool canS = (ycur >= lowerY[i+1] && ycur <= upperY[i+1]);
        bool canD = (ycur + 1 >= lowerY[i+1] && ycur + 1 <= upperY[i+1]);
        double xr = ax + i + 1;
        double yf = ay + (dx == 0 ? 0.0 : double(dy) * double(xr - ax) / double(dx));
        if (canS && !canD) { rote.push_back('S'); }
        else if (!canS && canD) { rote.push_back('D'); ++ycur; }
        else if (canS && canD) {
            if (std::abs(ycur - yf) <= std::abs((ycur+1) - yf)) rote.push_back('S');
            else { rote.push_back('D'); ++ycur; }
        } else {
            if (std::abs(ycur - yf) <= std::abs((ycur+1) - yf)) rote.push_back('S');
            else { rote.push_back('D'); ++ycur; }
        }
    }

    QVector<QPoint> pixels;
    pixels.reserve(rote.size() + 1);
    int tx = ax;
    double ystartf = ay;
    int ty = int(std::round(ystartf));
    pixels.append(QPoint(tx, ty));
    int ycur2 = ty;
    for (int i = 0; i < rote.size(); ++i) {
        char c = rote[i].toLatin1();
        if (c == 'S') { tx += 1; }
        else { tx += 1; ycur2 += 1; }
        if (pixels.isEmpty() || pixels.last() != QPoint(tx, ycur2))
            pixels.append(QPoint(tx, ycur2));
    }

    QVector<Step> outSteps;
    outSteps.reserve(pixels.size());
    for (QPoint p : pixels) {
        int rx = p.x(), ry = p.y();
        if (swapped_xy) swapInts(rx, ry);
        if (reflect_x) rx = -rx;
        if (reflect_y) ry = -ry;
        if (!outSteps.isEmpty()) {
            Point prev = outSteps.last().point();
            if (prev.x() == rx && prev.y() == ry) continue;
        }
        outSteps.append(makeStep(rx, ry, QString("rote:%1").arg(QString(rote)), 1.0));
    }

    res.setSteps(outSteps);
    res.setElapsed(high_resolution_clock::now() - t0);
    return res;
}

static double ipart(double x) { return std::floor(x); }
static double fpart(double x) { return x - std::floor(x); }
static double rfpart(double x) { return 1.0 - fpart(x); }

AlgorithmResult Algorithms::WuLine(const Point &A, const Point &B) {
    AlgorithmResult res;
    auto t0 = high_resolution_clock::now();

    double x0 = A.x(), y0 = A.y(), x1 = B.x(), y1 = B.y();
    QVector<Step> steps;

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    double dx = x1 - x0;
    double dy = y1 - y0;
    double gradient = (dx == 0.0) ? 0.0 : dy / dx;

    auto plot = [&](int x, int y, double intensity) {
        if (steep) appendIfNew(steps, makeStep(y, x, QString(), intensity));
        else appendIfNew(steps, makeStep(x, y, QString(), intensity));
    };

    double xend = std::round(x0);
    double yend = y0 + gradient * (xend - x0);
    double xgap = rfpart(x0 + 0.5);
    int xpxl1 = int(xend);
    int ypxl1 = int(ipart(yend));
    plot(xpxl1, ypxl1, rfpart(yend) * xgap);
    plot(xpxl1, ypxl1 + 1, fpart(yend) * xgap);
    double intery = yend + gradient;

    int xstart = xpxl1 + 1;
    int xend2 = int(std::round(x1));
    for (int x = xstart; x <= xend2 - 1; ++x) {
        double yf = intery;
        int yint = int(ipart(yf));
        plot(x, yint, rfpart(yf));
        plot(x, yint + 1, fpart(yf));
        intery += gradient;
    }

    double xend3 = std::round(x1);
    double yend2 = y1 + gradient * (xend3 - x1);
    double xgap2 = fpart(x1 + 0.5);
    int xpxl2 = int(xend3);
    int ypxl2 = int(ipart(yend2));
    plot(xpxl2, ypxl2, rfpart(yend2) * xgap2);
    plot(xpxl2, ypxl2 + 1, fpart(yend2) * xgap2);

    QVector<Step> compact;
    compact.reserve(steps.size());
    for (const Step &s : steps) {
        if (compact.isEmpty()) { compact.append(s); continue; }
        Point prev = compact.last().point();
        Point cur = s.point();
        if (prev.x() == cur.x() && prev.y() == cur.y()) {
            double prevI = compact.last().intensity();
            double newI = s.intensity();
            if (newI > prevI) compact.last().setIntensity(newI);
        } else compact.append(s);
    }

    res.setSteps(compact);
    res.setElapsed(high_resolution_clock::now() - t0);
    return res;
}

