#include "drawwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>

DrawWidget::DrawWidget(QWidget *parent)
    : QWidget(parent),
    m_currentIndex(-1),
    m_scale(24.0),
    m_pan(0,0) {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
}

void DrawWidget::setSteps(const QVector<Step> &s) {
    m_steps = s;
    m_currentIndex = s.isEmpty() ? -1 : s.size()-1;
    update();
}

void DrawWidget::clear() {
    m_steps.clear();
    m_currentIndex = -1;
    update();
}

void DrawWidget::advanceStep() {
    if (m_steps.isEmpty()) return;
    if (m_currentIndex < int(m_steps.size())-1) ++m_currentIndex;
    update();
}

void DrawWidget::resetStepIndex() {
    m_currentIndex = -1;
    update();
}

QPoint DrawWidget::lastClickedWorld() const {
    return m_lastClickWorld;
}

const QVector<Step>& DrawWidget::getSteps() const {
    return m_steps;
}

QPoint DrawWidget::worldToScreen(const QPoint &w) const {
    int sx = int(width()/2 + (w.x() + m_pan.x())*m_scale);
    int sy = int(height()/2 - (w.y() + m_pan.y())*m_scale);
    return QPoint(sx, sy);
}

QPoint DrawWidget::screenToWorld(const QPoint &s) const {
    double wx = (s.x() - width()/2)/m_scale - m_pan.x();
    double wy = (height()/2 - s.y())/m_scale - m_pan.y();
    return QPoint(int(std::round(wx)), int(std::round(wy)));
}

void DrawWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.fillRect(rect(), Qt::white);

    p.setPen(QPen(QColor(230,230,230)));
    int step = int(std::round(m_scale));
    QPoint center(width()/2, height()/2);

    if (step > 0) {
        for (int x = center.x()%step; x < width(); x += step)
            p.drawLine(x,0,x,height());
        for (int y = center.y()%step; y < height(); y += step)
            p.drawLine(0,y,width(),y);
    }

    p.setPen(QPen(Qt::black, 1));
    p.drawLine(0, center.y(), width(), center.y());
    p.drawLine(center.x(), 0, center.x(), height());

    const int tickSize = 4;
    QFont font = p.font();
    font.setPointSize(8);
    p.setFont(font);
    p.setPen(Qt::black);

    int xCount = width() / step / 2 + 2;
    for (int i = -xCount; i <= xCount; ++i) {
        int x = center.x() + i * step;
        p.drawLine(x, center.y() - tickSize, x, center.y() + tickSize);
        if (i != 0) {
            QString label = QString::number(i);
            QRect r(x - 12, center.y() + 6, 24, 12);
            p.drawText(r, Qt::AlignHCenter | Qt::AlignTop, label);
        }
    }

    int yCount = height() / step / 2 + 2;
    for (int j = -yCount; j <= yCount; ++j) {
        int y = center.y() - j * step;
        p.drawLine(center.x() - tickSize, y, center.x() + tickSize, y);
        if (j != 0) {
            QString label = QString::number(j);
            QRect r(center.x() + 6, y - 8, 24, 12);
            p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, label);
        }
    }

    // Подписи осей
    p.drawText(width() - 20, center.y() - 10, "X");
    p.drawText(center.x() + 6, 15, "Y");

    for (int i = 0; i < m_steps.size(); ++i) {
        Point pt = m_steps[i].point();
        double inten = m_steps[i].intensity();
        QPoint screen = worldToScreen(QPoint(pt.x(), pt.y()));
        int px = std::max(2, int(std::round(m_scale)));
        QRect r(screen.x()-px/2, screen.y()-px/2, px, px);

        QColor base = (i <= m_currentIndex) ? QColor(50,120,200) : QColor(180,200,240);
        p.fillRect(r, Qt::white);
        QColor fill = base;
        int alpha = int(qBound(0, int(std::round(inten * 255.0)), 255));
        fill.setAlpha(alpha);

        p.save();
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.fillRect(r, fill);
        p.restore();

        p.setPen(Qt::black);
        p.drawRect(r);
    }

    if (m_currentIndex >= 0 && m_currentIndex < m_steps.size()) {
        Point pt = m_steps[m_currentIndex].point();
        QPoint s = worldToScreen(QPoint(pt.x(), pt.y()));
        p.setPen(QPen(Qt::red, 2));
        int rsize = int(m_scale)+4;
        p.drawRect(s.x()-rsize/2, s.y()-rsize/2, rsize, rsize);
        p.drawText(s.x()+rsize/2+2, s.y(), m_steps[m_currentIndex].note());
    }
}

void DrawWidget::wheelEvent(QWheelEvent *ev) {
    double num = ev->angleDelta().y() / 120.0;
    double factor = std::pow(1.2, num);
    m_scale *= factor;
    if (m_scale < 6) m_scale = 6;
    if (m_scale > 240) m_scale = 240;
    update();
}

void DrawWidget::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        m_lastClickWorld = screenToWorld(ev->pos());
        emit setPointA();
    } else if (ev->button() == Qt::RightButton) {
        m_lastClickWorld = screenToWorld(ev->pos());
        emit setPointB();
    } else if (ev->button() == Qt::MiddleButton) {
        m_pan = QPointF(0,0);
        update();
    }
}
