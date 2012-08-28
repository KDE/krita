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
#include "KoReportItemBase.h"
#include <QRect>
#include <QPainter>
#include <QDomDocument>
#include "krpos.h"
#include "krsize.h"

#include "koreport_export.h"

namespace Scripting
{
class Line;
}

/**
 @author
*/
class KOREPORT_EXPORT  KoReportItemLine : public KoReportItemBase
{
public:
    KoReportItemLine() {
        createProperties();
    }
    KoReportItemLine(QDomNode & element);
    ~KoReportItemLine() {};

    virtual QString typeName() const;
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script);
    using KoReportItemBase::render;
    
    virtual void setUnit(const KoUnit&);

    KRPos startPosition() const;
    KRPos endPosition() const;

protected:
    KRPos m_start;
    KRPos m_end;
    KoProperty::Property *m_lineColor;
    KoProperty::Property *m_lineWeight;
    KoProperty::Property *m_lineStyle;

    KRLineStyleData lineStyle();
    unsigned int weight() const;
    void setWeight(int w);
    
private:
    virtual void createProperties();

    friend class Scripting::Line;
};

#endif
