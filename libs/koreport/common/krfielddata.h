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
#ifndef KRFIELDDATA_H
#define KRFIELDDATA_H
#include "krobjectdata.h"
#include <QRect>
#include <qdom.h>
#include "krsize.h"

/**
 @author
*/

namespace Scripting
{
class Field;
}

class KRFieldData : public KRObjectData
{
public:
    KRFieldData() {
        createProperties();
    };
    KRFieldData(QDomNode & element);
    ~KRFieldData();
    virtual KRFieldData * toField();
    virtual int type() const;

    Qt::Alignment textFlags() const;

    QFont font() const {
        return m_font->value().value<QFont>();
    }

    void setControlSource(const QString&);
    void setTrackTotal(bool);
    void setTrackTotalFormat(const QString &, bool = false);
    void setUseSubTotal(bool);

    QString controlSource() const;

    KRLineStyleData lineStyle();
    KRTextStyleData textStyle();
protected:

    KoProperty::Property * m_controlSource;
    KoProperty::Property * m_horizontalAlignment;
    KoProperty::Property * m_verticalAlignment;
    KoProperty::Property * m_font;
    //KoProperty::Property * m_trackTotal;
    //KoProperty::Property * m_trackBuiltinFormat;
    //KoProperty::Property * _useSubTotal;
    //KoProperty::Property * _trackTotalFormat;
    KoProperty::Property * m_foregroundColor;
    KoProperty::Property * m_backgroundColor;
    KoProperty::Property* m_backgroundOpacity;
    KoProperty::Property* m_lineColor;
    KoProperty::Property* m_lineWeight;
    KoProperty::Property* m_lineStyle;

    //bool builtinFormat;
    //QString format;

    QStringList fieldNames(const QString &);

private:
    virtual void createProperties();
    static int RTTI;

    friend class KoReportPreRendererPrivate;
    friend class Scripting::Field;
};

#endif
