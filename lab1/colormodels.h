#ifndef COLORMODELS_H
#define COLORMODELS_H

#include <QVector3D>

QVector3D rgbToXyz(const QVector3D &rgb);
QVector3D xyzToLab(const QVector3D &xyz);
QVector3D labToXyz(const QVector3D &lab);
QVector3D xyzToRgb(const QVector3D &xyz);

#endif // COLORMODELS_H


