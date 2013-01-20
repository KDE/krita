/*
   Calligra Report Engine
   Copyright (C) 2011, 2012 by Dag Andersen (danders@get2net.dk)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOODTFRAMESREPORTPICTURE_H
#define KOODTFRAMESREPORTPICTURE_H

#include "KoOdtFrameReportPrimitive.h"

#include <QString>

class KoXmlWriter;
class KoStore;
class OROPicture;
class OROPrimitive;

class KoOdtFrameReportPicture : public KoOdtFrameReportPrimitive
{
public:
    explicit KoOdtFrameReportPicture(OROPrimitive *primitive);
    virtual ~KoOdtFrameReportPicture();

    virtual void createBody(KoXmlWriter *bodyWriter) const;
    bool saveData(KoStore* store, KoXmlWriter* manifestWriter) const;

    OROPicture *picture() const;
    //NOTE: Store as png atm
    QString pictureName() const { return QString("Picture_%1.png").arg(m_uid); }
};

#endif // KOODTFRAMESREPORTPICTURE_H
