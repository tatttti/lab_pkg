#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include "imageinfo.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadImages();

private:
    QTableWidget *tableWidget;
    QPushButton *btnLoadImages;
    QLineEdit *folderPathEdit;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    void setupUI();
};

#endif // MAINWINDOW_H


