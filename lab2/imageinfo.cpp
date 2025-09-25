#include "imageinfo.h"
#include <QElapsedTimer>

#include <QFileInfo>
#include <QImageReader>

QString getCompressionInfo(const QString &format)
{
    QString f = format.toUpper();
    if (f == "JPG" || f == "JPEG") return "JPEG";
    if (f == "PNG") return "Deflate";
    if (f == "GIF") return "LZW";
    if (f == "BMP") return "Без сжатия";
    if (f == "TIF" || f == "TIFF") return "LZW/Deflate/JPEG";
    if (f == "PCX") return "RLE";
    return "Неизвестно";
}

/*QString getAdditionalInfo(const QString &format, const QImage &image)
{
    QString f = format.toUpper();
    if (f == "JPG" || f == "JPEG") return "YCbCr";
    if (f == "PNG") {
        if (!image.isNull()) {
            return QString("%1 каналов").arg(image.isGrayscale() ? "1 (grayscale)" :
                                                  image.hasAlphaChannel() ? "4 (RGBA)" : "3 (RGB)");
        }
    }
    if (f == "GIF") return "Палитровое изображение (до 256 цветов)";
    if (f == "BMP") return "Прямое кодирование цветов";
    return "-";
}*/

QString getAdditionalInfo(const QString &format, const QImage &image)
{
    QStringList details;
    QString f = format.toUpper();

    // Цветовое пространство
    if (f == "JPG" || f == "JPEG") {
        details << "YCbCr";
    } else if (f == "PNG" || f == "BMP") {
        details << "RGB";
    } else if (f == "GIF") {
        details << "Indexed";
    } else {
        details << "неизвестно";
    }

    // Тип изображения
    if (image.isGrayscale()) {
        details << "Grayscale";
    } else if (image.colorCount() > 0) {
        details << "Indexed";
    } else {
        details << "Truecolor";
    }

    // Каналы
    if (!image.isNull()) {
        if (image.isGrayscale()) {
            details << "1 канал";
        } else if (image.hasAlphaChannel()) {
            details << "4 канала (RGBA)";
            details << "Прозрачность есть";
        } else {
            details << "3 канала (RGB)";
            details << "Нет прозрачности";
        }
    } else {
        details << "Каналов: неизвестно";
    }



    // Соотношение сторон
    int w = image.width();
    int h = image.height();
    if (w > 0 && h > 0) {
        int gcd = std::gcd(w, h);
        details << QString("Соотношение: %1:%2").arg(w / gcd).arg(h / gcd);
    }

    return details.join(", ");
}


ImageInfo getImageInfo(const QString &filePath)
{
    ImageInfo info;
    QFileInfo fi(filePath);
    QImageReader reader(filePath);
    QImage image(filePath);

    info.fileName = fi.fileName();
    info.fileSize = QString("%1 KB").arg(fi.size() / 1024.0, 0, 'f', 1);
    info.format = reader.format().toUpper();

    QSize size = reader.size();
    if (size.isValid()) {
        info.size = QString("%1 x %2").arg(size.width()).arg(size.height());
    } else {
        info.size = image.isNull() ? "Некорректный размер" : QString("%1 x %2").arg(image.width()).arg(image.height());
    }

    int dpiX = static_cast<int>(image.dotsPerMeterX() * 0.0254 + 0.5);
    int dpiY = static_cast<int>(image.dotsPerMeterY() * 0.0254 + 0.5);
    QString ext = fi.suffix().toLower();

    //if (dpiX <= 0 || dpiY <= 0) fillDefaultDpi(ext, dpiX, dpiY);
    info.resolution = QString("%1 x %2").arg(dpiX).arg(dpiY);

    info.colorDepth = image.isNull() ? "Неизвестно" : QString("%1 бит").arg(image.depth());
    info.compression = getCompressionInfo(info.format);
    info.additionalInfo = getAdditionalInfo(info.format, image);

    return info;
}
