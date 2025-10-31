#include "mainwindow.h"
#include "drawwidget.h"
#include "algorithms.h"
#include "report.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentAlgoIndex(0) {
    createUi();
}

void MainWindow::createUi() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    drawWidget = new DrawWidget(this);
    drawWidget->setMinimumSize(720,520);

    runBtn = new QPushButton(tr("Run"), this);
    stepBtn = new QPushButton(tr("Step"), this);
    clearBtn = new QPushButton(tr("Clear"), this);
    exportBtn = new QPushButton(tr("Export Report"), this);

    algoBox = new QComboBox(this);
    algoBox->addItem("Step-by-step");
    algoBox->addItem("DDA");
    algoBox->addItem("Bresenham Line");
    algoBox->addItem("Bresenham Circle");
    algoBox->addItem("Kastl-Pitvey (Rote)");
    algoBox->addItem("Wu (antialiased)");


    axBox = new QSpinBox(this); ayBox = new QSpinBox(this);
    bxBox = new QSpinBox(this); byBox = new QSpinBox(this);
    axBox->setRange(-2000,2000); ayBox->setRange(-2000,2000);
    bxBox->setRange(-2000,2000); byBox->setRange(-2000,2000);
    axBox->setValue(0); ayBox->setValue(0); bxBox->setValue(0); byBox->setValue(0);

    showCalcBox = new QCheckBox(tr("Show calculations"), this);
    logEdit = new QTextEdit(this);
    logEdit->setReadOnly(true);
    statusLabel = new QLabel(tr("Ready"), this);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(new QLabel("Algorithm:"));
    rightLayout->addWidget(algoBox);
    rightLayout->addWidget(new QLabel("Point A (x,y):"));
    rightLayout->addWidget(axBox);
    rightLayout->addWidget(ayBox);
    rightLayout->addWidget(new QLabel("Point B (x,y) or radius.x for circle:"));
    rightLayout->addWidget(bxBox);
    rightLayout->addWidget(byBox);
    rightLayout->addWidget(showCalcBox);
    rightLayout->addWidget(runBtn);
    rightLayout->addWidget(stepBtn);
    rightLayout->addWidget(clearBtn);
    rightLayout->addWidget(exportBtn);
    rightLayout->addWidget(new QLabel("Log:"));
    rightLayout->addWidget(logEdit);
    rightLayout->addWidget(statusLabel);
    rightLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->addWidget(drawWidget, 1);
    mainLayout->addLayout(rightLayout);

    connect(runBtn, &QPushButton::clicked, this, &MainWindow::onRun);
    connect(stepBtn, &QPushButton::clicked, this, &MainWindow::onStep);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::onClear);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExportReport);
    connect(algoBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmChanged);

    connect(drawWidget, &DrawWidget::setPointA, this, &MainWindow::onSetPointA);
    connect(drawWidget, &DrawWidget::setPointB, this, &MainWindow::onSetPointB);
    connect(showCalcBox, &QCheckBox::toggled, this, [this](bool on){
        // передаём флаг виджету и обновляем лог/канву
        drawWidget->setShowCalculations(on);
        // если алгоритм уже выполнен — обновим лог
        if (on) {
            const auto &steps = drawWidget->getSteps();
            logEdit->clear();
            for (int i = 0; i < steps.size(); ++i)
                logEdit->append(QString("%1: (%2,%3) %4").arg(i).arg(steps[i].point().x()).arg(steps[i].point().y()).arg(steps[i].note()));
        } else {
            // возможно очистить лог или вернуть предыдущие сообщения
        }
    });

}

void MainWindow::onRun() {
    logEdit->clear();
    currentAlgoIndex = algoBox->currentIndex();
    Point A(axBox->value(), ayBox->value());
    Point B(bxBox->value(), byBox->value());
    AlgorithmResult res;
    if (currentAlgoIndex == 0) res = Algorithms::stepByStep(A,B);
    else if (currentAlgoIndex == 1) res = Algorithms::DDA(A,B);
    else if (currentAlgoIndex == 2) res = Algorithms::BresenhamLine(A,B);
    else if (currentAlgoIndex == 3) res = Algorithms::BresenhamCircle(A,B);
    else if (currentAlgoIndex == 4) res = Algorithms::KastlPitvey(A,B);
    else if (currentAlgoIndex == 5) res = Algorithms::WuLine(A,B);


    // Установка шагов в виджет и сброс индекса для пошаговой отрисовки
    drawWidget->setSteps(res.steps());
    drawWidget->resetStepIndex();
    drawWidget->update();

    // Всегда пишем краткую сводку
    logEdit->clear();
    logEdit->append(QString("Steps: %1").arg(res.steps().size()));
    logEdit->append(QString("Elapsed ms: %1").arg(res.elapsed().count()*1000.0, 0, 'f', 3));
    statusLabel->setText("Done");

    // Дополнительно: вывод подробных вычислений в лог, если чекбокс включён
    if (showCalcBox->isChecked()) {
        logEdit->append("\nDetailed steps:");
        const auto &steps = res.steps();
        for (int i = 0; i < steps.size(); ++i) {
            // Для совместимости с вашей реализацией Step используйте point() и note()
            Point p = steps[i].point();
            QString note = steps[i].note();
            // Если Step имеет intensity (для Wu), можно добавить её:
            // double inten = steps[i].intensity();
            logEdit->append(QString("%1: (%2, %3) %4").arg(i).arg(p.x()).arg(p.y()).arg(note));
        }
    }


}

void MainWindow::onStep() {
    drawWidget->advanceStep();
}

void MainWindow::onClear() {
    drawWidget->clear();
    logEdit->clear();
    statusLabel->setText("Cleared");
}

void MainWindow::onExportReport() {
    Report r = Report::fromWidget(drawWidget);
    QString fname = QFileDialog::getSaveFileName(this, tr("Save report"), QString(), tr("Text files (*.txt)"));
    if (fname.isEmpty()) return;
    if (!r.saveToFile(fname)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save report"));
    } else {
        statusLabel->setText("Report saved");
    }
}

void MainWindow::onAlgorithmChanged(int index) {
    Q_UNUSED(index);
}

void MainWindow::onSetPointA() {
    QPoint p = drawWidget->lastClickedWorld();
    axBox->setValue(p.x()); ayBox->setValue(p.y());
}

void MainWindow::onSetPointB() {
    QPoint p = drawWidget->lastClickedWorld();
    bxBox->setValue(p.x()); byBox->setValue(p.y());
}
