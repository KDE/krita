/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoCellValue.h"

#include <KoXmlWriter.h>

#include <QMap>

KoCellValue::KoCellValue()
{
}

KoCellValue::~KoCellValue()
{
}

void KoCellValue::saveOdf(KoXmlWriter& writer) const
{
    QString stringType;

    if(!type().isEmpty()) {
        writer.addAttribute("office:value-type", type());
    }

    typedef QPair<QString, QString> Attribute;
    foreach(const Attribute &attribute, attributes()) {
        //TODO is this safe? I think that it didn't use to be
        writer.addAttribute(attribute.first.toLatin1(), attribute.second);
    }
}
