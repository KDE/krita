/*
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
#include "krscriptmaps.h"
#include "KoReportItemMaps.h"
#include <QBuffer>
#include <kcodecs.h>
#include <kdebug.h>

namespace Scripting
{

Maps::Maps(KoReportItemMaps *i)
{
    m_map = i;
}


Maps::~Maps()
{
}

QPointF Maps::position()
{
    return m_map->m_pos.toPoint();
}
void Maps::setPosition(const QPointF& p)
{
    m_map->m_pos.setPointPos(p);
}

QSizeF Maps::size()
{
    return m_map->m_size.toPoint();
}
void Maps::setSize(const QSizeF& s)
{
    m_map->m_size.setPointSize(s);
}

QString Maps::resizeMode()
{
    return m_map->m_resizeMode->value().toString();
}

void Maps::setResizeMode(const QString &rm)
{
    if (rm == "Stretch") {
        m_map->m_resizeMode->setValue("Stretch");
    } else {
        m_map->m_resizeMode->setValue("Clip");
    }
}

void Maps::setInlineImage(const QByteArray &ba)
{
    //m_map->setInlineImageData(ba);
}

void Maps::loadFromFile(const QVariant &pth)
{
    QPixmap img;

    QString str = pth.toString();
    //m_map->setInlineImageData(QByteArray(), str);
}
}
