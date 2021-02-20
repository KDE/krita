/*
    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
    SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
    SPDX-FileCopyrightText: 2009 Jan Hambrecht <jaham@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <resources/KoAbstractGradient.h>
#include "KoColorSpaceRegistry.h"

#include <KoColor.h>

#include <QBuffer>
#include <QByteArray>

#define PREVIEW_WIDTH 2048
#define PREVIEW_HEIGHT 1


struct Q_DECL_HIDDEN KoAbstractGradient::Private {
    const KoColorSpace* colorSpace;
    QGradient::Spread spread;
    QGradient::Type type;
};

KoAbstractGradient::KoAbstractGradient(const QString& filename)
        : KoResource(filename)
        , d(new Private)
{
    d->colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    d->spread = QGradient::PadSpread;
    d->type = QGradient::NoGradient;
}

KoAbstractGradient::~KoAbstractGradient()
{
    delete d;
}

KoAbstractGradient::KoAbstractGradient(const KoAbstractGradient &rhs)
    : KoResource(rhs)
    , d(new Private(*rhs.d))
{
}

KoAbstractGradientSP KoAbstractGradient::cloneAndBakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface) const
{
    KoAbstractGradientSP result = this->clone().dynamicCast<KoAbstractGradient>();
    if (canvasResourcesInterface) {
        result->bakeVariableColors(canvasResourcesInterface);
    }
    return result;
}

void KoAbstractGradient::bakeVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    Q_UNUSED(canvasResourcesInterface);
}

KoAbstractGradientSP KoAbstractGradient::cloneAndUpdateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface) const
{
    KoAbstractGradientSP result = this->clone().dynamicCast<KoAbstractGradient>();
    if (canvasResourcesInterface) {
        result->updateVariableColors(canvasResourcesInterface);
    }
    return result;
}

void KoAbstractGradient::updateVariableColors(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    Q_UNUSED(canvasResourcesInterface);
}

void KoAbstractGradient::colorAt(KoColor&, qreal t) const
{
    Q_UNUSED(t);
}

void KoAbstractGradient::setColorSpace(KoColorSpace* colorSpace)
{
    d->colorSpace = colorSpace;
}

const KoColorSpace* KoAbstractGradient::colorSpace() const
{
    return d->colorSpace;
}

void KoAbstractGradient::setSpread(QGradient::Spread spreadMethod)
{
    d->spread = spreadMethod;
}

QGradient::Spread KoAbstractGradient::spread() const
{
    return d->spread;
}

void KoAbstractGradient::setType(QGradient::Type repeatType)
{
    d->type = repeatType;
}

QGradient::Type KoAbstractGradient::type() const
{
    return d->type;
}

QImage KoAbstractGradient::generatePreview(int width, int height) const
{
    QImage image(width, height, QImage::Format_ARGB32);

    QRgb * firstLine = reinterpret_cast<QRgb*>(image.scanLine(0));

    KoColor c;
    QColor color;
    // first create a reference line
    for (int x = 0; x < image.width(); ++x) {

        qreal t = static_cast<qreal>(x) / (image.width() - 1);
        colorAt(c, t);
        c.toQColor(&color);

        firstLine[x] = color.rgba();
    }

    int bytesPerLine = image.bytesPerLine();

    // now copy lines accordingly
    for (int y = 1; y < image.height(); ++y) {
        QRgb * line = reinterpret_cast<QRgb*>(image.scanLine(y));

        memcpy(line, firstLine, bytesPerLine);
    }

    return image;
}

QImage KoAbstractGradient::generatePreview(int width, int height, KoCanvasResourcesInterfaceSP canvasResourcesInterface) const
{
    QImage result;

    if (!requiredCanvasResources().isEmpty()) {
        KoAbstractGradientSP gradient = cloneAndBakeVariableColors(canvasResourcesInterface);
        result = gradient->generatePreview(width, height);
    } else {
        result = generatePreview(width, height);
    }

    return result;
}

void KoAbstractGradient::updatePreview()
{
    setImage(generatePreview(PREVIEW_WIDTH, PREVIEW_HEIGHT));
}
