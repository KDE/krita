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
#include "krscriptbarcode.h"

namespace Scripting
{

Barcode::Barcode(KRBarcodeData *b)
{
    m_barcode = b;
}


Barcode::~Barcode()
{
}

QPointF Barcode::position()
{
    return m_barcode->m_pos.toPoint();
}
void Barcode::setPosition(const QPointF& p)
{
    m_barcode->m_pos.setPointPos(p);
}

QSizeF Barcode::size()
{
    return m_barcode->m_size.toPoint();
}
void Barcode::setSize(const QSizeF& s)
{
    m_barcode->m_size.setPointSize(s);
}

int Barcode::horizontalAlignment()
{
    QString a = m_barcode->m_horizontalAlignment->value().toString();

    if (a.toLower() == "left") {
        return -1;
    } else if (a.toLower() == "center") {
        return 0;
    } else if (a.toLower() == "right") {
        return 1;
    }
    return -1;
}
void Barcode::setHorizonalAlignment(int a)
{
    switch (a) {
    case -1:
        m_barcode->m_horizontalAlignment->setValue("left");
        break;
    case 0:
        m_barcode->m_horizontalAlignment->setValue("center");
        break;
    case 1:
        m_barcode->m_horizontalAlignment->setValue("right");
        break;
    default:
        m_barcode->m_horizontalAlignment->setValue("left");
        break;
    }
}

QString Barcode::source()
{
    return m_barcode->m_controlSource->value().toString();
}

void Barcode::setSource(const QString& s)
{
    m_barcode->m_controlSource->setValue(s);
}

QString Barcode::format()
{
    return m_barcode->m_format->value().toString();
}

void Barcode::setFormat(const QString& s)
{
    m_barcode->m_format->setValue(s);
}
}
