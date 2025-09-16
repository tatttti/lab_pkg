#include "mainwindow.h"
#include "colormodels.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QColorDialog>
#include <QMessageBox>
#include <QFrame>
#include <QFont>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , updating(false)
{
    setupUI();
    connectSignals();
    updateFromRgb();
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout *mainLay = new QHBoxLayout(central);
    mainLay->setContentsMargins(40, 20, 40, 20);
    mainLay->setSpacing(50);

    QWidget *leftW = new QWidget(this);
    QVBoxLayout *leftLay = new QVBoxLayout(leftW);
    leftLay->setSpacing(20);

    titleLabel = new QLabel("Цветовые модели RGB ↔ XYZ ↔ LAB", this);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Arial", 18, QFont::Bold));
    leftLay->addWidget(titleLabel);

    leftLay->addSpacing(40);
    QRegularExpression intRegex("-?\\d+");

    auto makeSection = [&](const QString &title,
                           QSlider* sliders[3],
                           QLineEdit* edits[3],
                           int minVal, int maxVal,
                           const QStringList &labels)
    {
        QGroupBox *grp = new QGroupBox(title, this);
        QGridLayout *grid = new QGridLayout(grp);
        grid->setVerticalSpacing(10);

        for (int i = 0; i < 3; ++i) {
            grid->addWidget(new QLabel(labels[i], this), i, 0);

            sliders[i] = new QSlider(Qt::Horizontal, this);
            sliders[i]->setRange(minVal, maxVal);
            grid->addWidget(sliders[i], i, 1);

            edits[i] = new QLineEdit(this);
            edits[i]->setValidator(new QRegularExpressionValidator(intRegex, this));
            edits[i]->setFixedWidth(60);
            grid->addWidget(edits[i], i, 2);
        }
        return grp;
    };

    leftLay->addWidget(makeSection("RGB", rgbSliders, rgbEdits, 0, 255, {"R","G","B"}));
    leftLay->addWidget(makeSection("XYZ", xyzSliders, xyzEdits, 0, 100, {"X","Y","Z"}));

    QGroupBox *labGrp = makeSection("Lab", labSliders, labEdits, 0, 100, {"L","a","b"});
    for (int i = 1; i < 3; ++i) {
        labSliders[i]->setRange(-128, 127);
    }
    leftLay->addWidget(labGrp);

    paletteButton = new QPushButton("Выбрать цвет...", this);
    paletteButton->setFixedHeight(50);
    paletteButton->setFont(QFont("Arial", 14));
    leftLay->addWidget(paletteButton);

    warningLabel = new QLabel(this);
    warningLabel->setStyleSheet("color: orange;");
    warningLabel->setAlignment(Qt::AlignCenter);
    warningLabel->setVisible(false);
    leftLay->addWidget(warningLabel);

    leftLay->addStretch(1);
    mainLay->addWidget(leftW, 0);

    QWidget *rightW = new QWidget(this);
    QVBoxLayout *rightLay = new QVBoxLayout(rightW);
    rightLay->addStretch(1);

    previewLabel = new QLabel(this);
    previewLabel->setFrameShape(QFrame::Box);
    previewLabel->setAutoFillBackground(true);
    previewLabel->setMinimumSize(600, 700);
    rightLay->addWidget(previewLabel, 0, Qt::AlignCenter);

    rightLay->addStretch(1);
    mainLay->addWidget(rightW, 1);
}

void MainWindow::connectSignals() {
    connect(paletteButton, &QPushButton::clicked, this, &MainWindow::pickColor);

    for (int i = 0; i < 3; ++i) {
        connect(rgbSliders[i], &QSlider::valueChanged,   this, &MainWindow::onRgbSlider);
        connect(xyzSliders[i], &QSlider::valueChanged,   this, &MainWindow::onXyzSlider);
        connect(labSliders[i], &QSlider::valueChanged,   this, &MainWindow::onLabSlider);

        connect(rgbEdits[i],   &QLineEdit::editingFinished, this, &MainWindow::onRgbEdit);
        connect(xyzEdits[i],   &QLineEdit::editingFinished, this, &MainWindow::onXyzEdit);
        connect(labEdits[i],   &QLineEdit::editingFinished, this, &MainWindow::onLabEdit);
    }
}

void MainWindow::onRgbSlider(int) {
    if (!updating) updateFromRgb();
}

void MainWindow::onXyzSlider(int) {
    if (!updating) updateFromXyz();
}

void MainWindow::onLabSlider(int) {
    if (!updating) updateFromLab();
}

void MainWindow::onRgbEdit() {
    auto *e = qobject_cast<QLineEdit*>(sender());
    int idx = -1;
    for (int i = 0; i < 3; ++i) {
        if (e == rgbEdits[i]) { idx = i; break; }
    }
    if (idx < 0) return;

    bool ok;
    int v = e->text().toInt(&ok);
    if (!ok) return;

    if (v < 0 || v > 255) {
        QMessageBox::warning(
            this,
            "Неверное RGB-значение",
            QString("Компонента %1 должна быть от 0 до 255.\n"
                    "Введено %2, будет скорректировано.")
                .arg(idx==0?"R":idx==1?"G":"B")
                .arg(v)
            );
        v = qBound(0, v, 255);
    }

    updating = true;
    rgbSliders[idx]->setValue(v);
    rgbEdits[idx]->setText(QString::number(v));
    updating = false;

    updateFromRgb();
}

void MainWindow::onXyzEdit() {
    auto *e = qobject_cast<QLineEdit*>(sender());
    int idx = -1;
    for (int i = 0; i < 3; ++i) {
        if (e == xyzEdits[i]) { idx = i; break; }
    }
    if (idx < 0) return;

    bool ok;
    int v = e->text().toInt(&ok);
    if (!ok) return;

    if (v < 0 || v > 100) {
        QMessageBox::warning(
            this,
            "Неверное XYZ-значение",
            QString("Компонента %1 должна быть от 0 до 100.\n"
                    "Введено %2, будет скорректировано.")
                .arg(QString("XYZ")[idx])
                .arg(v)
            );
        v = qBound(0, v, 100);
    }
    updating = true;
    xyzSliders[idx]->setValue(v);
    xyzEdits[idx]->setText(QString::number(v));
    updating = false;
    updateFromXyz();
}

void MainWindow::onLabEdit() {
    auto *e = qobject_cast<QLineEdit*>(sender());
    int idx = -1;
    for (int i = 0; i < 3; ++i) {
        if (e == labEdits[i]) { idx = i; break; }
    }
    if (idx < 0) return;

    bool ok;
    int v = e->text().toInt(&ok);
    if (!ok) return;

    int minV = (idx==0 ? 0 : -128);
    int maxV = (idx==0 ? 100 : 127);
    if (v < minV || v > maxV) {
        QMessageBox::warning(
            this,
            QString("Неверное значение %1").arg(idx==0?"L":idx==1?"a":"b"),
            QString("Компонента должна быть от %1 до %2.\n" "Введено %3, будет скорректировано.")
                .arg(minV).arg(maxV).arg(v)
            );
        v = qBound(minV, v, maxV);
    }

    updating = true;
    labSliders[idx]->setValue(v);
    labEdits[idx]->setText(QString::number(v));
    updating = false;
    updateFromLab();
}

void MainWindow::updatePreview(const QVector3D &rgb) {
    warningLabel->setVisible(false);
    QPalette pal = previewLabel->palette();
    pal.setColor(QPalette::Window, QColor(int(rgb.x()), int(rgb.y()), int(rgb.z())));
    previewLabel->setPalette(pal);
}

void MainWindow::pickColor() {
    QColor c = QColorDialog::getColor();
    if (!c.isValid()) return;

    updating = true;
    rgbSliders[0]->setValue(c.red());
    rgbSliders[1]->setValue(c.green());
    rgbSliders[2]->setValue(c.blue());
    updating = false;
    updateFromRgb();
}

void MainWindow::updateFromRgb() {
    updating = true;

    int r = rgbSliders[0]->value(),
        g = rgbSliders[1]->value(),
        b = rgbSliders[2]->value();
    rgbEdits[0]->setText(QString::number(r));
    rgbEdits[1]->setText(QString::number(g));
    rgbEdits[2]->setText(QString::number(b));

    QVector3D rgb(r, g, b);
    QVector3D xyz = rgbToXyz(rgb);
    QVector3D lab = xyzToLab(xyz);

    for (int i = 0; i < 3; ++i) {
        double val = (i==0?xyz.x(): i==1?xyz.y(): xyz.z());
        int vi = qBound(0, int(val + 0.5), 100);
        if (vi != int(val + 0.5))
            QMessageBox::warning(this, "Клиппинг XYZ", "XYZ-компонента обрезана до [0,100].");
        xyzSliders[i]->setValue(vi);
        xyzEdits[i]->setText(QString::number(vi));
    }

    double lv = lab.x(), av = lab.y(), bv = lab.z();
    int Li = qBound(0, int(lv + 0.5), 100);
    int ai = qBound(-128, int(av + 0.5), 127);
    int bi = qBound(-128, int(bv + 0.5), 127);
    if (Li != int(lv + 0.5) || ai != int(av + 0.5) || bi != int(bv + 0.5))
        QMessageBox::warning(this, "Клиппинг Lab", "Lab-компонента обрезана до допустимых границ.");
    labSliders[0]->setValue(Li);
    labSliders[1]->setValue(ai);
    labSliders[2]->setValue(bi);
    labEdits[0]->setText(QString::number(Li));
    labEdits[1]->setText(QString::number(ai));
    labEdits[2]->setText(QString::number(bi));

    updatePreview(rgb);
    updating = false;
}

void MainWindow::updateFromXyz() {
    updating = true;

    int xi = xyzSliders[0]->value(),
        yi = xyzSliders[1]->value(),
        zi = xyzSliders[2]->value();
    xyzEdits[0]->setText(QString::number(xi));
    xyzEdits[1]->setText(QString::number(yi));
    xyzEdits[2]->setText(QString::number(zi));

    QVector3D xyz(xi, yi, zi);
    QVector3D lab = xyzToLab(xyz);
    QVector3D rgb = xyzToRgb(xyz);

    double lv = lab.x(), av = lab.y(), bv = lab.z();
    int Li = qBound(0, int(lv + 0.5), 100);
    int ai = qBound(-128, int(av + 0.5), 127);
    int bi = qBound(-128, int(bv + 0.5), 127);
    if (Li != int(lv + 0.5) || ai != int(av + 0.5) || bi != int(bv + 0.5))
        QMessageBox::warning(this, "Клиппинг Lab", "Lab-компонента обрезана до допустимых границ.");
    labSliders[0]->setValue(Li);
    labSliders[1]->setValue(ai);
    labSliders[2]->setValue(bi);
    labEdits[0]->setText(QString::number(Li));
    labEdits[1]->setText(QString::number(ai));
    labEdits[2]->setText(QString::number(bi));

    double rv = rgb.x(), gv = rgb.y(), bv2 = rgb.z();
    int ri = qBound(0, int(rv + 0.5), 255);
    int gi = qBound(0, int(gv + 0.5), 255);
    int biRgb = qBound(0, int(bv2 + 0.5), 255);
    if (ri != int(rv + 0.5) || gi != int(gv + 0.5) || biRgb != int(bv2 + 0.5))
        QMessageBox::warning(this, "Клиппинг RGB", "RGB-компонента обрезана до [0,255].");
    rgbSliders[0]->setValue(ri);
    rgbSliders[1]->setValue(gi);
    rgbSliders[2]->setValue(biRgb);
    rgbEdits[0]->setText(QString::number(ri));
    rgbEdits[1]->setText(QString::number(gi));
    rgbEdits[2]->setText(QString::number(biRgb));

    updatePreview(rgb);
    updating = false;
}

void MainWindow::updateFromLab() {
    updating = true;

    int Li = labSliders[0]->value(), ai = labSliders[1]->value(), bi = labSliders[2]->value();
    labEdits[0]->setText(QString::number(Li));
    labEdits[1]->setText(QString::number(ai));
    labEdits[2]->setText(QString::number(bi));

    QVector3D lab(Li, ai, bi);
    QVector3D xyz = labToXyz(lab);
    QVector3D rgb = xyzToRgb(xyz);

    for (int i = 0; i < 3; ++i) {
        double val = (i==0? xyz.x(): i==1? xyz.y(): xyz.z());
        int vi = qBound(0, int(val + 0.5), 100);
        if (vi != int(val + 0.5))
            QMessageBox::warning(this, "Клиппинг XYZ", "XYZ-компонента обрезана до [0,100].");
        xyzSliders[i]->setValue(vi);
        xyzEdits[i]->setText(QString::number(vi));
    }

    double rv = rgb.x(), gv = rgb.y(), bv3 = rgb.z();
    int ri = qBound(0, int(rv + 0.5), 255);
    int gi = qBound(0, int(gv + 0.5), 255);
    int biRgb = qBound(0, int(bv3 + 0.5), 255);
    if (ri != int(rv + 0.5) || gi != int(gv + 0.5) || biRgb != int(bv3 + 0.5))
        QMessageBox::warning(this, "Клиппинг RGB", "RGB-компонента обрезана до [0,255].");
    rgbSliders[0]->setValue(ri);
    rgbSliders[1]->setValue(gi);
    rgbSliders[2]->setValue(biRgb);
    rgbEdits[0]->setText(QString::number(ri));
    rgbEdits[1]->setText(QString::number(gi));
    rgbEdits[2]->setText(QString::number(biRgb));

    updatePreview(rgb);
    updating = false;
}


