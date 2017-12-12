/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "HtmlSavingContext.h"
#include <KoXmlWriter.h>
#include <KoShape.h>
#include <QBuffer>

struct HtmlSavingContext::Private {

    Private(QIODevice *_shapeDevice)
        : shapeDevice(_shapeDevice)
        , shapeWriter(0)
    {
        shapeWriter.reset(new KoXmlWriter(&shapeBuffer, 1));
    }

    QIODevice *shapeDevice;
    QBuffer shapeBuffer;
    QScopedPointer<KoXmlWriter> shapeWriter;
};

HtmlSavingContext::HtmlSavingContext(QIODevice &shapeDevice)
    : d(new Private(&shapeDevice))
{
}

HtmlSavingContext::~HtmlSavingContext()
{
    d->shapeDevice->write(d->shapeBuffer.data());
}

KoXmlWriter &HtmlSavingContext::shapeWriter()
{
    return *d->shapeWriter;
}
