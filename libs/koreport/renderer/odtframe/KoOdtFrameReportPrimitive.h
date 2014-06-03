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

#ifndef KOODTFRAMESREPORTPRIMITIVE_H
#define KOODTFRAMESREPORTPRIMITIVE_H

#include <QString>

class KoGenStyles;
class KoXmlWriter;
class OROPrimitive;
class KoStore;

class KoOdtFrameReportPrimitive
{
public:
    explicit KoOdtFrameReportPrimitive(OROPrimitive *primitive = 0);
    virtual ~KoOdtFrameReportPrimitive();

    bool isValid() const;
    void setPrimitive(OROPrimitive *primitive);
    /// @note OROPrimitive page starts at 0, odt starts at 1
    int pageNumber() const;
    void setUID(int uid);
    int uid() const;

    virtual void createStyle(KoGenStyles&);
    virtual void createBody(KoXmlWriter *bodyWriter) const;
    virtual bool saveData(KoStore *store, KoXmlWriter*) const;

protected:
    QString itemName() const;
    void commonAttributes(KoXmlWriter *bodyWriter) const;

protected:
    OROPrimitive *m_primitive;
    int m_uid;
    QString m_frameStyleName;

};

#endif // KOODTFRAMESREPORTPRIMITIVE_H
