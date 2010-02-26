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
#ifndef KRLINEDATA_H
#define KRLINEDATA_H
#include "krobjectdata.h"
#include <QRect>
#include <QPainter>
#include <qdom.h>
#include "krpos.h"
#include "krsize.h"

namespace Scripting
{
class Line;
}

/**
 @author
*/
class KRLineData : public KRObjectData
{
public:
    KRLineData() {
        createProperties();
    }
    KRLineData(QDomNode & element);
    ~KRLineData() {};
    virtual int type() const;
    virtual KRLineData * toLine();
    KRLineStyleData lineStyle();
    unsigned int weight() const;
    void setWeight(int w);

protected:
    KRPos m_start;
    KRPos m_end;
    KoProperty::Property *m_lineColor;
    KoProperty::Property *m_lineWeight;
    KoProperty::Property *m_lineStyle;
    virtual void createProperties();
private:

    static int RTTI;

    friend class KoReportPreRendererPrivate;
    friend class Scripting::Line;
};

#endif
