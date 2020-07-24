/*
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
    Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
