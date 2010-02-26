/*
 * Kexi Report Plugin
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krscriptimage.h"
#include <krimagedata.h>
#include <QBuffer>
#include <kcodecs.h>
#include <kdebug.h>

namespace Scripting
{

Image::Image(KRImageData *i)
{
    m_image = i;
}


Image::~Image()
{
}

QPointF Image::position()
{
    return m_image->m_pos.toPoint();
}
void Image::setPosition(const QPointF& p)
{
    m_image->m_pos.setPointPos(p);
}

QSizeF Image::size()
{
    return m_image->m_size.toPoint();
}
void Image::setSize(const QSizeF& s)
{
    m_image->m_size.setPointSize(s);
}

QString Image::resizeMode()
{
    return m_image->m_resizeMode->value().toString();
}

void Image::setResizeMode(const QString &rm)
{
    if (rm == "Stretch") {
        m_image->m_resizeMode->setValue("Stretch");
    } else {
        m_image->m_resizeMode->setValue("Clip");
    }
}

void Image::setInlineImage(const QByteArray &ba)
{
    m_image->setInlineImageData(ba);
}

void Image::loadFromFile(const QVariant &pth)
{
    QPixmap img;

    QString str = pth.toString();
    m_image->setInlineImageData(QByteArray(), str);
}
}
