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

#include <krobjectdata.h>
#include <QRect>
#include <QPainter>
#include <qdom.h>
#include "krpos.h"
#include "krsize.h"

namespace Scripting
{
class Check;
}

class KRCheckData : public KRObjectData
{
public:
    KRCheckData() {
        createProperties();
    };
    KRCheckData(QDomNode &element);
    virtual ~KRCheckData();
    virtual int type() const;
    virtual KRCheckData * toCheck();
    bool value();
    void setValue(bool);
    KRLineStyleData lineStyle();

    QString controlSource()const;

protected:

    KoProperty::Property * m_controlSource;
    KoProperty::Property* m_checkStyle;
    KoProperty::Property* m_foregroundColor;
    KoProperty::Property* m_lineColor;
    KoProperty::Property* m_lineWeight;
    KoProperty::Property* m_lineStyle;

private:
    virtual void createProperties();
    static int RTTI;

    friend class Scripting::Check;
    friend class KoReportPreRendererPrivate;
};

#endif // KRCHECKDATA_H
