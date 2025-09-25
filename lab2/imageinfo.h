#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <QString>
#include <QImage>

struct ImageInfo {
    QString fileName;
    QString size;
    QString resolution;
    QString colorDepth;
    QString compression;
    QString format;
    QString fileSize;
    QString additionalInfo;
};

ImageInfo getImageInfo(const QString &filePath);

#endif // IMAGEINFO_H

