#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QPixmap>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void saveImage();
    void applyLinearContrast();
    void applyEqualization();
    void applySharpen();

private:
    QImage originalImage;
    QImage processedImage;

    QLabel *labelOriginal;
    QLabel *labelProcessed;
    QLabel *labelHistOriginal;
    QLabel *labelHistProcessed;

    QPushButton *btnLoad;
    QPushButton *btnSave;
    QPushButton *btnLinear;
    QPushButton *btnEqualize;
    QPushButton *btnSharpen;

    QComboBox *comboMode;

    void setupUI();
    void displayImages();
    QPixmap generateHistogram(const QImage &img);
};

#endif // MAINWINDOW_H

