#include "report.h"
#include "drawwidget.h"
#include <QFile>
#include <QTextStream>

Report::Report() : m_title(""), m_steps(), m_elapsedMs(0.0) {}

void Report::setTitle(const QString &t) { m_title = t; }
QString Report::title() const { return m_title; }

void Report::setSteps(const QVector<Step> &s) { m_steps = s; }
QVector<Step> Report::steps() const { return m_steps; }

void Report::setElapsedMs(double v) { m_elapsedMs = v; }
double Report::elapsedMs() const { return m_elapsedMs; }

QString Report::toString() const {
    QString s;
    s += "Raster Algorithms Report\n";
    s += "Title: " + m_title + "\n";
    s += QString("Steps: %1\n").arg(m_steps.size());
    s += QString("Elapsed ms: %1\n").arg(m_elapsedMs, 0, 'f', 3);
    s += "Steps detail:\n";
    for (int i = 0; i < m_steps.size(); ++i) {
        Point p = m_steps[i].point();
        s += QString("%1: (%2,%3) %4\n").arg(i).arg(p.x()).arg(p.y()).arg(m_steps[i].note());
    }
    return s;
}

bool Report::saveToFile(const QString &path) const {
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return false;
    QTextStream out(&f);
    out << toString();
    f.close();
    return true;
}

Report Report::fromWidget(const DrawWidget *w) {
    Report r;
    r.setTitle("From DrawWidget");
    r.setSteps(w->getSteps());
    r.setElapsedMs(0.0);
    return r;
}

Report Report::fromWidget(DrawWidget *w) { return fromWidget((const DrawWidget*)w); }
