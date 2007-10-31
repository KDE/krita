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

#include <QGradient>

#include "KoColor.h"
#include "KoColorSpace.h"
#include "KoResource.h"
#include <koresource_export.h>

class KORESOURCES_EXPORT KoAbstractGradient : public KoResource {
    Q_OBJECT

public:
    KoAbstractGradient(const QString& filename);
    virtual ~KoAbstractGradient();

    virtual bool load() { return false; }
    virtual bool save() { return false; }

    virtual QGradient* toQGradient() const { return new QGradient(); }

    void setColorSpace(KoColorSpace* colorSpace);
    KoColorSpace * colorSpace() const;

    void setSpread(QGradient::Spread spreadMethod);
    QGradient::Spread spread() const;

    void setType(QGradient::Type repeatType);
    QGradient::Type type() const;

private:
    struct Private;
    Private* const d;
};

#endif // KOABSTRACTGRADIENT_H
