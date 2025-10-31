#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QPoint>

class Point {
public:
    Point() : m_x(0), m_y(0) {}
    Point(int x, int y) : m_x(x), m_y(y) {}
    int x() const { return m_x; }
    int y() const { return m_y; }
    void setX(int v) { m_x = v; }
    void setY(int v) { m_y = v; }
private:
    int m_x;
    int m_y;
};

class Step {
public:
    Step() : m_point(), m_note(), m_intensity(1.0) {}
    Step(const Point &p, const QString &note = QString(), double intensity = 1.0)
        : m_point(p), m_note(note), m_intensity(intensity) {}
    Point point() const { return m_point; }
    QString note() const { return m_note; }
    double intensity() const { return m_intensity; }
    void setPoint(const Point &p) { m_point = p; }
    void setNote(const QString &n) { m_note = n; }
    void setIntensity(double v) { m_intensity = qBound(0.0, v, 1.0); }
private:
    Point m_point;
    QString m_note;
    double m_intensity;
};

class DrawWidget : public QWidget {
    Q_OBJECT
public:
    explicit DrawWidget(QWidget *parent = nullptr);
    ~DrawWidget() override = default;
    void setSteps(const QVector<Step>& s);
    void clear();
    void advanceStep();
    void resetStepIndex();
    QPoint lastClickedWorld() const;
    const QVector<Step>& getSteps() const;
    void setShowCalculations(bool v) { m_showCalc = v; update(); }
    bool showCalculations() const { return m_showCalc; }
protected:
    void paintEvent(QPaintEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
private:

    QVector<Step> m_steps;
    int m_currentIndex;
    double m_scale;
    QPointF m_pan;
    QPoint m_lastClickWorld;
    QPoint worldToScreen(const QPoint &w) const;
    QPoint screenToWorld(const QPoint &s) const;
    bool m_showCalc = false;

signals:
    void setPointA();
    void setPointB();
};

#endif // DRAWWIDGET_H
