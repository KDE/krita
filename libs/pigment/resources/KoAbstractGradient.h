/*
    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

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
#ifndef KOABSTRACTGRADIENT_H
#define KOABSTRACTGRADIENT_H

#include <QtGui/QGradient>

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoResource.h"
#include <pigment_export.h>

/**
 * KoAbstractGradient is the base class of all gradient resources
 */
class PIGMENTCMS_EXPORT KoAbstractGradient : public KoResource {

public:
    KoAbstractGradient(const QString& filename);
    virtual ~KoAbstractGradient();

    virtual bool load() { return false; }
    virtual bool save() { return false; }

    /**
    * Creates a QGradient from the gradient.
    * The resulting QGradient might differ from original gradient
    */
    virtual QGradient* toQGradient() const { return new QGradient(); }

    /// gets the color at position 0 <= t <= 1
    virtual void colorAt(KoColor&, qreal t) const;

    void setColorSpace(KoColorSpace* colorSpace);
    const KoColorSpace * colorSpace() const;

    void setSpread(QGradient::Spread spreadMethod);
    QGradient::Spread spread() const;

    void setType(QGradient::Type repeatType);
    QGradient::Type type() const;

    QImage image() const;
    void updatePreview();

    QImage generatePreview(int width, int height) const;

private:
    struct Private;
    Private* const d;
};

#endif // KOABSTRACTGRADIENT_H
