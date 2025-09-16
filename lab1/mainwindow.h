#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVector3D>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QLabel *titleLabel;
    QLabel *previewLabel;
    QPushButton *paletteButton;
    QLabel *warningLabel;

    QSlider *rgbSliders[3];
    QLineEdit *rgbEdits[3];
    QSlider *xyzSliders[3];
    QLineEdit *xyzEdits[3];
    QSlider *labSliders[3];
    QLineEdit *labEdits[3];

    bool updating = false;

    void setupUI();
    void connectSignals();
    void updateFromRgb();
    void updateFromXyz();
    void updateFromLab();
    void updatePreview(const QVector3D &rgb);
    void showWarning(const QString &text);

private slots:
    void onRgbSlider(int);
    void onRgbEdit();
    void onXyzSlider(int);
    void onXyzEdit();
    void onLabSlider(int);
    void onLabEdit();
    void pickColor();
};

#endif // MAINWINDOW_H
