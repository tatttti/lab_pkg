#include "imageprocessor.h"
#include <QColor>
#include <algorithm>
#include <cmath>

QImage ImageProcessor::linearContrast(const QImage& img) {
    QImage result = img.copy();
    int minVal = 255, maxVal = 0;

    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            int gray = qGray(img.pixel(x, y));
            minVal = std::min(minVal, gray);
            maxVal = std::max(maxVal, gray);
        }

    if (maxVal == minVal) return img;

    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QColor c = img.pixelColor(x, y);
            int r = (c.red() - minVal) * 255 / (maxVal - minVal);
            int g = (c.green() - minVal) * 255 / (maxVal - minVal);
            int b = (c.blue() - minVal) * 255 / (maxVal - minVal);
            result.setPixelColor(x, y, QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255)));
        }
    }

    return result;
}

QVector<int> ImageProcessor::computeHistogram(const QImage& img, int channel) {
    QVector<int> hist(256, 0);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            QColor c = img.pixelColor(x, y);
            int val = (channel == 0) ? c.red() : (channel == 1) ? c.green() : c.blue();
            hist[val]++;
        }
    return hist;
}

QVector<uchar> ImageProcessor::computeEqualizationLUT(const QVector<int>& hist, int totalPixels) {
    QVector<uchar> lut(256);
    QVector<int> cdf(256);
    cdf[0] = hist[0];

    for (int i = 1; i < 256; ++i)
        cdf[i] = cdf[i - 1] + hist[i];

    int cdfMin = *std::find_if(cdf.begin(), cdf.end(), [](int v) { return v > 0; });

    for (int i = 0; i < 256; ++i)
        lut[i] = qBound(0, (cdf[i] - cdfMin) * 255 / (totalPixels - cdfMin), 255);

    return lut;
}

QImage ImageProcessor::equalizeRGB(const QImage& img) {
    QImage result(img.size(), img.format());
    int totalPixels = img.width() * img.height();

    QVector<uchar> lutR = computeEqualizationLUT(computeHistogram(img, 0), totalPixels);
    QVector<uchar> lutG = computeEqualizationLUT(computeHistogram(img, 1), totalPixels);
    QVector<uchar> lutB = computeEqualizationLUT(computeHistogram(img, 2), totalPixels);

    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QColor c = img.pixelColor(x, y);
            int r = lutR[c.red()];
            int g = lutG[c.green()];
            int b = lutB[c.blue()];
            result.setPixelColor(x, y, QColor(r, g, b));
        }
    }

    return result;
}

QImage ImageProcessor::equalizeHSV(const QImage& img) {
    QImage result = img.copy();
    int totalPixels = img.width() * img.height();

    QVector<int> hist(256, 0);
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            int v = img.pixelColor(x, y).toHsv().value();
            hist[v]++;
        }

    QVector<uchar> lut = computeEqualizationLUT(hist, totalPixels);

    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            QColor hsv = img.pixelColor(x, y).toHsv();
            int newV = lut[hsv.value()];
            newV = qBound(0, (int)(newV * 1.2), 255);
            hsv.setHsv(hsv.hue(), hsv.saturation(), newV);
            result.setPixelColor(x, y, hsv.toRgb());
        }
    }

    return result;
}

QImage ImageProcessor::sharpen(const QImage& img) {
    QImage result = img.copy();
    int kernel[3][3] = {
        { 0, -1,  0 },
        {-1,  5, -1 },
        { 0, -1,  0 }
    };

    for (int y = 1; y < img.height() - 1; ++y) {
        for (int x = 1; x < img.width() - 1; ++x) {
            int r = 0, g = 0, b = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    QColor c = img.pixelColor(x + kx, y + ky);
                    int k = kernel[ky + 1][kx + 1];
                    r += c.red() * k;
                    g += c.green() * k;
                    b += c.blue() * k;
                }
            }
            result.setPixelColor(x, y, QColor(qBound(0, r, 255), qBound(0, g, 255), qBound(0, b, 255)));
        }
    }

    return result;
}

