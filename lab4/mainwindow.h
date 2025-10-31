#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class DrawWidget;
class QPushButton;
class QComboBox;
class QSpinBox;
class QLabel;
class QTextEdit;
class QCheckBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;
private slots:
    void onRun();
    void onStep();
    void onClear();
    void onExportReport();
    void onAlgorithmChanged(int index);
    void onSetPointA();
    void onSetPointB();
private:
    void createUi();
    DrawWidget *drawWidget;
    QPushButton *runBtn;
    QPushButton *stepBtn;
    QPushButton *clearBtn;
    QPushButton *exportBtn;
    QComboBox *algoBox;
    QSpinBox *axBox;
    QSpinBox *ayBox;
    QSpinBox *bxBox;
    QSpinBox *byBox;
    QTextEdit *logEdit;
    QCheckBox *showCalcBox;
    QLabel *statusLabel;
    int currentAlgoIndex;
};

#endif // MAINWINDOW_H

