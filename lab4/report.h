#ifndef REPORT_H
#define REPORT_H

#include <QString>
#include <QVector>

class Step;
class DrawWidget;

class Report {
public:
    Report();
    ~Report() = default;

    void setTitle(const QString &t);
    QString title() const;

    void setSteps(const QVector<class Step> &s);
    QVector<class Step> steps() const;

    void setElapsedMs(double v);
    double elapsedMs() const;

    QString toString() const;
    bool saveToFile(const QString &path) const;

    static Report fromWidget(const DrawWidget *w);
    static Report fromWidget(DrawWidget *w);

private:
    QString m_title;
    QVector<class Step> m_steps;
    double m_elapsedMs;
};

#endif // REPORT_H

