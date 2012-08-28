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

#ifndef KOODTFRAMESREPORTLINE_H
#define KOODTFRAMESREPORTLINE_H

#include "KoOdtFrameReportPrimitive.h"

#include <QString>

class KoGenStyles;
class KoXmlWriter;
class OROLine;
class OROPrimitive;

// FIXME:
// This implementation is a hack because neither words nor libre writer
// implements lines in a good way.

class KoOdtFrameReportLine : public KoOdtFrameReportPrimitive
{
public:
    KoOdtFrameReportLine(OROLine *primitive);
    virtual ~KoOdtFrameReportLine();

    virtual void createStyle(KoGenStyles &coll);
    virtual void createBody(KoXmlWriter *bodyWriter) const;

    OROLine *line() const;

protected:
    void frameStyle(KoGenStyles &coll);

private:
    QString m_frameStyleName;
    QString m_paragraphStyleName;
    QString m_textStyleName;
};

#endif // KOODTFRAMESREPORTLINE_H
