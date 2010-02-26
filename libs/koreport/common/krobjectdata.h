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
#ifndef KROBJECTDATA_H
#define KROBJECTDATA_H
#include <koproperty/Property.h>
#include <QDomElement>
#include <QFont>
#include <QColor>
#include "krpos.h"
#include "krsize.h"

class KRSize;
class KRLineData;
class KRLabelData;
class KRFieldData;
class KRTextData;
class KRBarcodeData;
class KRImageData;
class KRChartData;
class KRShapeData;
class KRCheckData;

namespace KoProperty
{
class Set;
class Property;
}

class KRTextStyleData
{
public:
    QFont font;
    Qt::Alignment alignment;
    QColor backgroundColor;
    QColor foregroundColor;
    int backgroundOpacity;

};

class KRLineStyleData
{
public:
    int weight;
    QColor lineColor;
    Qt::PenStyle style;
};


/**
 @author
*/
class KRObjectData
{
public:
    enum EntityTypes {
        EntityNone = 0,
        EntityLine  = 65537,
        EntityLabel = 65550,
        EntityField = 65551,
        EntityText  = 65552,
        EntityBarcode = 65553,
        EntityImage = 65554,
        EntityChart = 65555,
        EntityShape = 65556,
        EntityCheck = 65557,
        EntityLast = 65558
    };

    KRObjectData();
    virtual ~KRObjectData();

    virtual int type() const = 0;
    virtual KRLineData * toLine();
    virtual KRLabelData * toLabel();
    virtual KRFieldData * toField();
    virtual KRTextData * toText();
    virtual KRBarcodeData * toBarcode();
    virtual KRImageData * toImage();
    virtual KRChartData * toChart();
    virtual KRShapeData * toShape();
    virtual KRCheckData * toCheck();

    KoProperty::Set* properties() {
        return m_set;
    }
    virtual void createProperties() = 0;

    qreal Z;
    KRPos position() {
        return m_pos;
    }

    void setEntityName(const QString& n) {
        m_name->setValue(n);
    }
    QString entityName() {
        return m_name->value().toString();
    }
protected:
    KoProperty::Set *m_set;
    KoProperty::Property *m_name;
    KRPos m_pos;
    KRSize m_size;
    
    QString m_oldName;

    void addDefaultProperties();

    static bool parseReportRect(const QDomElement &, KRPos *pos, KRSize *siz);
    static bool parseReportTextStyleData(const QDomElement &, KRTextStyleData &);
    static bool parseReportLineStyleData(const QDomElement &, KRLineStyleData &);


};

#endif
