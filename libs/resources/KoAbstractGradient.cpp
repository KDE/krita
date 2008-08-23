/*
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KoAbstractGradient.h"
#include "KoColorSpaceRegistry.h"

#define PREVIEW_WIDTH 64
#define PREVIEW_HEIGHT 64


struct KoAbstractGradient::Private {
    const KoColorSpace* colorSpace;
    QGradient::Spread spread;
    QGradient::Type type;
    QImage img;
};

KoAbstractGradient::KoAbstractGradient(const QString& filename)
    : KoResource(filename)
    , d(new Private)
{
    d->colorSpace = KoColorSpaceRegistry::instance()->rgb8();
}

KoAbstractGradient::~KoAbstractGradient()
{
   delete d;
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

QGradient::Spread KoAbstractGradient::spread () const
{
    return d->spread;
}

void KoAbstractGradient::setType(QGradient::Type repeatType)
{
    d->type = repeatType;
}

QGradient::Type KoAbstractGradient::type () const
{
    return d->type;
}

QImage KoAbstractGradient::generatePreview(int width, int height) const
{
    QImage img(width, height, QImage::Format_RGB32);

    KoColor c;
    QColor color;
    for (int x = 0; x < img.width(); x++) {

        qreal t = static_cast<qreal>(x) / (img.width() - 1);
        colorAt(c, t);
        c.toQColor( &color );
        qreal alpha = static_cast<qreal>(color.alpha()) / OPACITY_OPAQUE;

        for (int y = 0; y < img.height(); y++) {
            int backgroundRed = 128 + 63 * ((x / 4 + y / 4) % 2);
            int backgroundGreen = backgroundRed;
            int backgroundBlue = backgroundRed;

            int red = static_cast<int>((1 - alpha) * backgroundRed + alpha * color.red() + 0.5);
            int green = static_cast<int>((1 - alpha) * backgroundGreen + alpha * color.green() + 0.5);
            int blue = static_cast<int>((1 - alpha) * backgroundBlue + alpha * color.blue() + 0.5);

            img.setPixel(x, y, qRgb(red, green, blue));
        }
    }

    return img;
}

QImage KoAbstractGradient::img() const
{
    return d->img;
}

void KoAbstractGradient::updatePreview()
{
    d->img = generatePreview( PREVIEW_WIDTH, PREVIEW_HEIGHT );
}


#include "KoAbstractGradient.moc"
