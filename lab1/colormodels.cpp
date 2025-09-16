#include "colormodels.h"
#include <cmath>
#include <algorithm>

QVector3D rgbToXyz(const QVector3D &rgb) {
    double r = rgb.x() / 255.0;
    double g = rgb.y() / 255.0;
    double b = rgb.z() / 255.0;

    auto gamma = [](double c) {
        return (c > 0.04045) ? std::pow((c + 0.055) / 1.055, 2.4) : (c / 12.92);
    };

    r = gamma(r);
    g = gamma(g);
    b = gamma(b);

    double x = r * 0.4124 + g * 0.3576 + b * 0.1805;
    double y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    double z = r * 0.0193 + g * 0.1192 + b * 0.9505;

    return { x * 100.0, y * 100.0, z * 100.0 };
}

QVector3D xyzToLab(const QVector3D &xyz) {
    constexpr double Xn = 95.047, Yn = 100.000, Zn = 108.883;
    double x = xyz.x() / Xn;
    double y = xyz.y() / Yn;
    double z = xyz.z() / Zn;

    auto f = [](double t) {
        return (t > 0.008856) ? std::cbrt(t) : (7.787 * t + 16.0 / 116.0);
    };

    double fx = f(x);
    double fy = f(y);
    double fz = f(z);

    double L = 116.0 * fy - 16.0;
    double a = 500.0 * (fx - fy);
    double b = 200.0 * (fy - fz);

    return { L, a, b };
}

QVector3D labToXyz(const QVector3D &lab) {
    constexpr double Xn = 95.047, Yn = 100.000, Zn = 108.883;
    double L = lab.x(), a = lab.y(), b = lab.z();

    double fy = (L + 16.0) / 116.0;
    double fx = a / 500.0 + fy;
    double fz = fy - b / 200.0;

    auto finv = [](double t) {
        double t3 = t * t * t;
        return (t3 > 0.008856) ? t3 : ((t - 16.0/116.0) / 7.787);
    };

    double xr = finv(fx);
    double yr = finv(fy);
    double zr = finv(fz);

    return { xr * Xn, yr * Yn, zr * Zn };
}

QVector3D xyzToRgb(const QVector3D &xyz) {
    double x = xyz.x() / 100.0;
    double y = xyz.y() / 100.0;
    double z = xyz.z() / 100.0;

    double r = x *  3.2406 + y * -1.5372 + z * -0.4986;
    double g = x * -0.9689 + y *  1.8758 + z *  0.0415;
    double b = x *  0.0557 + y * -0.2040 + z *  1.0570;

    auto invGamma = [](double c) {
        return (c > 0.0031308) ? (1.055 * std::pow(c, 1.0/2.4) - 0.055) : (12.92 * c);
    };

    r = invGamma(r);
    g = invGamma(g);
    b = invGamma(b);

    r = std::clamp(r, 0.0, 1.0);
    g = std::clamp(g, 0.0, 1.0);
    b = std::clamp(b, 0.0, 1.0);

    return { r * 255.0, g * 255.0, b * 255.0 };
}

