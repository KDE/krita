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
#include <KoReportItemBase.h>
#include <QRect>
#include <QDomDocument>
#include <krsize.h>

/**
 @author
*/

namespace Scripting
{
class Field;
}

class KoReportItemField : public KoReportItemBase
{
public:
    KoReportItemField() {
        createProperties();
    };
    KoReportItemField(QDomNode & element);
    virtual ~KoReportItemField();

    virtual QString typeName() const;
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script);
    using KoReportItemBase::render;

    virtual QString itemDataSource() const;

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
    KoProperty::Property* m_wordWrap;
    KoProperty::Property* m_canGrow;
    
    //bool builtinFormat;
    //QString format;

    QStringList fieldNames(const QString &);

    KRLineStyleData lineStyle();
    KRTextStyleData textStyle();

    void setTrackTotal(bool);
    void setTrackTotalFormat(const QString &, bool = false);
    void setUseSubTotal(bool);
    int textFlags() const;

    QFont font() const {
        return m_font->value().value<QFont>();
    }
    
    void setItemDataSource(const QString&);
    
private:
    virtual void createProperties();
    
    friend class Scripting::Field;
};

#endif
