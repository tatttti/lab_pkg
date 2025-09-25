
#include "mainwindow.h"
#include "imageinfo.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QElapsedTimer>
#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    showMaximized();
    setWindowTitle("📁 Image Info Scanner");
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    setWindowTitle("Image Information");

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    QLabel *folderLabel = new QLabel("Папка:", this);
    folderPathEdit = new QLineEdit(this);
    folderPathEdit->setReadOnly(true);

    btnLoadImages = new QPushButton("Загрузить папку", this);
    btnLoadImages->setStyleSheet(R"(
        QPushButton {
            background-color: #3498db;
            color: white;
            padding: 8px 16px;
            font-weight: bold;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
    )");

    controlLayout->addWidget(folderLabel);
    controlLayout->addWidget(folderPathEdit, 1);
    controlLayout->addWidget(btnLoadImages);

    tableWidget = new QTableWidget(this);
    tableWidget->setColumnCount(8);
    tableWidget->setHorizontalHeaderLabels(QStringList()
                                           << "Имя файла" << "Размер (пиксели)" << "Разрешение (DPI)"
                                           << "Глубина цвета" << "Сжатие" << "Формат" << "Размер файла" << "Доп. информация");

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->setAlternatingRowColors(true);

    QFont tableFont("Segoe UI", 11);
    tableWidget->setFont(tableFont);

    tableWidget->setStyleSheet(R"(
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #f2f2f2;
            gridline-color: #d0d0d0;
            selection-background-color: #cce5ff;
            selection-color: black;
        }
        QHeaderView::section {
            background-color: #dfe6e9;
            font-weight: bold;
            padding: 6px;
            border: 1px solid #ccc;
        }
    )");

    // Фиксированная ширина колонок
    tableWidget->setColumnWidth(0, 200); // Имя файла
    tableWidget->setColumnWidth(1, 120); // Размер (пиксели)
    tableWidget->setColumnWidth(2, 120); // DPI
    tableWidget->setColumnWidth(3, 100); // Глубина цвета
    tableWidget->setColumnWidth(4, 150); // Сжатие
    tableWidget->setColumnWidth(5, 80);  // Формат
    tableWidget->setColumnWidth(6, 100);  // Размер файла
    tableWidget->setColumnWidth(7, 280); // Доп. информация

    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    progressBar->setStyleSheet("QProgressBar { height: 20px; }");

    statusLabel = new QLabel("Готов к работе", this);
    statusLabel->setStyleSheet("QLabel { font-style: italic; color: #555; padding: 4px; }");

    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(tableWidget, 1);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(statusLabel);

    setCentralWidget(centralWidget);
    setStyleSheet("QMainWindow { background-color: #eaeff2; }");

    connect(btnLoadImages, &QPushButton::clicked, this, &MainWindow::onLoadImages);
}

void MainWindow::onLoadImages()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку", QDir::homePath());
    if (folder.isEmpty()) return;

    folderPathEdit->setText(folder);

    QStringList formats = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif", "*.tif", "*.tiff", "*.pcx"};
    QDirIterator it(folder, formats, QDir::Files, QDirIterator::Subdirectories);
    QStringList files;
    while (it.hasNext() && files.size() < 100000) files.append(it.next());

    if (files.isEmpty()) {
        QMessageBox::information(this, "Информация", "В выбранной папке нет изображений!");
        return;
    }

    tableWidget->setRowCount(0);
    progressBar->setVisible(true);
    progressBar->setRange(0, files.size());
    progressBar->setValue(0);
    btnLoadImages->setEnabled(false);

    QElapsedTimer timer;
    timer.start();

    int processed = 0;
    for (const QString &filePath : files) {
        ImageInfo info = getImageInfo(filePath);

        int row = tableWidget->rowCount();
        tableWidget->insertRow(row);

        QStringList data = {
            info.fileName, info.size, info.resolution, info.colorDepth,
            info.compression, info.format, info.fileSize, info.additionalInfo
        };

        for (int i = 0; i < data.size(); ++i) {
            QTableWidgetItem* item = new QTableWidgetItem(data[i]);
            item->setTextAlignment(i == 0 || i == 7 ? Qt::AlignLeft : Qt::AlignCenter);
            tableWidget->setItem(row, i, item);
        }

        processed++;
        progressBar->setValue(processed);
        if (processed % 10 == 0) QApplication::processEvents();
    }

    progressBar->setVisible(false);
    btnLoadImages->setEnabled(true);

    qint64 elapsedMs = timer.elapsed();
    statusLabel->setText(QString("Обработано %1 файлов за %2 мс").arg(files.size()).arg(elapsedMs));
}
