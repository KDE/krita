/*
 * Kexi report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
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

//
// KRSectionData is used to store the information about a specific
// section.
// A section has a name and optionally extra data. `name'
// reportheader, reportfooter, pageheader, pagefooter, groupheader, groupfooter or detail.
// In the case of pghead and pgfoot extra would contain the page
// designation (firstpage, odd, even or lastpage).
//
#ifndef KRSECTIONDATA_H
#define KRSECTIONDATA_H

#include <QObject>
#include <qdom.h>

class KRObjectData;

#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <QColor>

namespace Scripting
{
class Section;
}
class KRSectionData : public QObject
{
    Q_OBJECT
public:
    enum Section {
        None = 0,
        PageHeaderFirst = 1,
        PageHeaderOdd,
        PageHeaderEven,
        PageHeaderLast,
        PageHeaderAny,
        ReportHeader,
        ReportFooter,
        PageFooterFirst,
        PageFooterOdd,
        PageFooterEven,
        PageFooterLast,
        PageFooterAny,
        GroupHeader,
        GroupFooter,
        Detail
    };

    KRSectionData();
    KRSectionData(const QDomElement &);
    ~KRSectionData();
    KoProperty::Set* properties() {
        return m_set;
    }

    bool isValid() const {
        return m_valid;
    }

    qreal height() const {
        return m_height->value().toDouble();
    }

    QList<KRObjectData*> objects() const {
        return m_objects;
    }

    QString name() const;

    QColor backgroundColor() const {
        return m_backgroundColor->value().value<QColor>();
    }

    Section type() const {
        return m_type;
    }

    static KRSectionData::Section sectionTypeFromString(const QString& s);
    static QString sectionTypeString(KRSectionData::Section s);
protected:
    KoProperty::Set *m_set;
    KoProperty::Property *m_height;
    KoProperty::Property *m_backgroundColor;

public slots:
    KoProperty::Set& propertySet() {
        return *m_set;
    }

private:
    void createProperties();

    QList<KRObjectData*> m_objects;

    QString m_name;
    Section m_type;

    static bool zLessThan(KRObjectData* s1, KRObjectData* s2);
    static bool xLessThan(KRObjectData* s1, KRObjectData* s2);

    bool m_valid;

    friend class Scripting::Section;
    friend class ReportSection;
};

#endif
