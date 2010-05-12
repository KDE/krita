/*
 * KoReport Library
 * Copyright (C) 2007-2010 by Adam Pigg (adam@piggz.co.uk)
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
#include <QObject>

#include <QDomElement>
#include <QFont>
#include <QColor>
#include "koreport_export.h"

#include "krpos.h"
#include "krsize.h"


class OROPage;
class OROSection;
class KRSize;
class KRScriptHandler;

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
class KOREPORT_EXPORT KoReportItemBase : public QObject
{
    Q_OBJECT
public:

    KoReportItemBase();
    virtual ~KoReportItemBase();

    virtual QString typeName() const = 0;
    

    /**
    @brief Render the item into a primitive which is used by the second stage renderer
    @return the height required by the object
    */
    virtual int render(OROPage* page, OROSection* section,  QPointF offset, QVariant data, KRScriptHandler *script) = 0;

    //!Override if the item supports data
    virtual QString itemDataSource() const {return QString();}

    KoProperty::Set* properties() {
        return m_set;
    }

    KRPos position() {
        return m_pos;
    }

    void setEntityName(const QString& n) {
        m_name->setValue(n);
    }
    QString entityName() {
        return m_name->value().toString();
    }

    virtual void setUnit(const KoUnit& u);

    qreal Z;
protected:
    KoProperty::Set *m_set;
    KoProperty::Property *m_name;
    KRPos m_pos;
    KRSize m_size;
    
    QString m_oldName;

    void addDefaultProperties();

    virtual void createProperties() = 0;
    
    static bool parseReportRect(const QDomElement &, KRPos *pos, KRSize *siz);
    static bool parseReportTextStyleData(const QDomElement &, KRTextStyleData &);
    static bool parseReportLineStyleData(const QDomElement &, KRLineStyleData &);


};

#endif
