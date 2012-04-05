/*
 * Kexi Report Plugin
 * Copyright (C) 2009-2010 by Adam Pigg (adam@piggz.co.uk)
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

#ifndef KRCHECKDATA_H
#define KRCHECKDATA_H

#include <KoReportItemBase.h>
#include <QRect>
#include <QPainter>
#include <QDomDocument>
#include "krpos.h"
#include "krsize.h"

namespace Scripting
{
class Check;
}

class KoReportItemCheck : public KoReportItemBase
{
public:
    KoReportItemCheck() {
        createProperties();
    };
    KoReportItemCheck(QDomNode &element);
    virtual ~KoReportItemCheck();

    virtual QString typeName() const;
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script);
    using KoReportItemBase::render;

    virtual QString itemDataSource()const;

protected:

    KoProperty::Property * m_controlSource;
    KoProperty::Property* m_checkStyle;
    KoProperty::Property* m_foregroundColor;
    KoProperty::Property* m_lineColor;
    KoProperty::Property* m_lineWeight;
    KoProperty::Property* m_lineStyle;

    bool value();
    void setValue(bool);
    KRLineStyleData lineStyle();
    
private:
    virtual void createProperties();
    bool m_value;
    
    friend class Scripting::Check;
};

#endif // KRCHECKDATA_H
