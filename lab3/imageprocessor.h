#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>
#include <QVector>

class ImageProcessor {
public:
    static QImage linearContrast(const QImage& img);
    static QImage equalizeRGB(const QImage& img);
    static QImage equalizeHSV(const QImage& img);
    static QImage sharpen(const QImage& img);

private:
    static QVector<int> computeHistogram(const QImage& img, int channel);
    static QVector<uchar> computeEqualizationLUT(const QVector<int>& hist, int totalPixels);
};

#endif // IMAGEPROCESSOR_H
