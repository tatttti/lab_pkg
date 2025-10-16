#include "mainwindow.h"
#include "imageprocessor.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QPixmap>
#include <QFrame>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    labelOriginal = new QLabel("Оригинал");
    labelProcessed = new QLabel("Результат");
    labelHistOriginal = new QLabel("Гистограмма до");
    labelHistProcessed = new QLabel("Гистограмма после");

    QList<QLabel*> labels = {labelOriginal, labelProcessed, labelHistOriginal, labelHistProcessed};
    for (auto lbl : labels) {
        lbl->setFrameStyle(QFrame::Box | QFrame::Plain);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        lbl->setStyleSheet("background-color: #f0f0f0; border-radius: 8px;");
    }

    btnLoad = new QPushButton("Загрузить изображение");
    btnSave = new QPushButton("Сохранить результат");
    btnLinear = new QPushButton("Линейное контрастирование");
    btnEqualize = new QPushButton("Эквализация гистограммы");
    btnSharpen = new QPushButton("Повышение резкости");

    comboMode = new QComboBox();
    comboMode->addItem("RGB (каждая компонента)");
    comboMode->addItem("HSV (только яркость)");

    QList<QWidget*> controls = {btnLoad, btnSave, btnLinear, btnEqualize, comboMode, btnSharpen};
    for (auto w : controls) {
        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        w->setStyleSheet(
            "QPushButton, QComboBox {"
            "background-color: #0078d7;"
            "color: white;"
            "border-radius: 6px;"
            "padding: 6px;"
            "font: 14px 'Segoe UI';"
            "}"
            "QPushButton:hover, QComboBox:hover {"
            "background-color: #005a9e;"
            "}"
            );
    }

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(labelOriginal, 0, 0);
    gridLayout->addWidget(labelProcessed, 0, 1);
    gridLayout->addWidget(labelHistOriginal, 1, 0);
    gridLayout->addWidget(labelHistProcessed, 1, 1);
    gridLayout->setRowStretch(0, 4);
    gridLayout->setRowStretch(1, 2);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setSpacing(15);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(btnLoad);
    buttonLayout->addWidget(btnSave);
    buttonLayout->addWidget(btnLinear);
    buttonLayout->addWidget(btnEqualize);
    buttonLayout->addWidget(comboMode);
    buttonLayout->addWidget(btnSharpen);
    buttonLayout->setSpacing(10);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(gridLayout);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    central->setLayout(mainLayout);

    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveImage);
    connect(btnLinear, &QPushButton::clicked, this, &MainWindow::applyLinearContrast);
    connect(btnEqualize, &QPushButton::clicked, this, &MainWindow::applyEqualization);
    connect(btnSharpen, &QPushButton::clicked, this, &MainWindow::applySharpen);
}

void MainWindow::loadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть изображение", "", "Images (*.png *.jpg *.bmp *.jpeg)");
    if (!fileName.isEmpty()) {
        if (!originalImage.load(fileName)) return;
        originalImage = originalImage.convertToFormat(QImage::Format_RGB32);
        processedImage = originalImage;
        displayImages();
    }
}

void MainWindow::saveImage()
{
    if (processedImage.isNull()) return;
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "PNG (*.png);;JPEG (*.jpg *.jpeg);;BMP (*.bmp)");
    if (!fileName.isEmpty()) processedImage.save(fileName);
}

void MainWindow::applyLinearContrast() {
    if (originalImage.isNull()) return;
    processedImage = ImageProcessor::linearContrast(originalImage);
    displayImages();
}

void MainWindow::applyEqualization() {
    if (originalImage.isNull()) return;
    QString mode = comboMode->currentText();
    if (mode.startsWith("RGB"))
        processedImage = ImageProcessor::equalizeRGB(originalImage);
    else
        processedImage = ImageProcessor::equalizeHSV(originalImage);
    displayImages();
}

void MainWindow::applySharpen() {
    if (originalImage.isNull()) return;
    processedImage = ImageProcessor::sharpen(originalImage);
    displayImages();
}

void MainWindow::displayImages()
{
    if (!originalImage.isNull()) {
        labelOriginal->setPixmap(QPixmap::fromImage(originalImage).scaled(labelOriginal->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        labelHistOriginal->setPixmap(generateHistogram(originalImage));
    } else {
        labelOriginal->clear();
        labelHistOriginal->clear();
    }

    if (!processedImage.isNull()) {
        labelProcessed->setPixmap(QPixmap::fromImage(processedImage).scaled(labelProcessed->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        labelHistProcessed->setPixmap(generateHistogram(processedImage));
    } else {
        labelProcessed->clear();
        labelHistProcessed->clear();
    }
}

QPixmap MainWindow::generateHistogram(const QImage &img)
{
    if (img.isNull()) return QPixmap();

    QVector<int> hist(256, 0);
    for (int y = 0; y < img.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            int gray = qGray(line[x]);
            hist[gray]++;
        }
    }

    int maxVal = *std::max_element(hist.begin(), hist.end());
    if (maxVal == 0) maxVal = 1;

    QPixmap pix(256, 120);
    pix.fill(Qt::white);
    QPainter p(&pix);
    p.setPen(Qt::black);
    for (int i = 0; i < 256; ++i) {
        int h = hist[i] * 100 / maxVal;
        p.drawLine(i, 110, i, 110 - h);
    }
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
    return pix;
}
