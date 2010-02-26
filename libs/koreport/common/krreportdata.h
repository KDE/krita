/*
 * Kexi Report Plugin /
 * OpenRPT report writer and rendering engine
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

#ifndef KRREPORTDATA_H
#define KRREPORTDATA_H

#include <QObject>
#include <qdom.h>
#include "krsectiondata.h"
#include "reportpageoptions.h"

class KRDetailSectionData;

namespace Scripting
{
class Report;
}
/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class KRReportData : public QObject
{
    Q_OBJECT

public:
    explicit KRReportData(const QDomElement & elemSource);
    KRReportData();
    ~KRReportData();

    bool isValid() const {
        return m_valid;
    }

    /**
    \return a list of all objects in the report
    */
    QList<KRObjectData*> objects() const;

    /**
    \return a report object given its name
    */
    KRObjectData* object(const QString&) const;

    /**
    \return all the sections, including groups and detail
    */
    QList<KRSectionData*> sections() const;

    /**
    \return a sectiondata given a section enum
    */
    KRSectionData* section(KRSectionData::Section) const;

    /**
    \return a sectiondata given its name
    */
    KRSectionData* section(const QString&) const;

    QString query() const {
        return m_query;
    }
    QString script() const {
        return m_script;
    };
    QString interpreter() const {
        return m_interpreter;
    }

    bool externalData() const {
        return m_externalData;
    }

    KRDetailSectionData* detail() const {
        return m_detailSection;
    }

    void setName(const QString&n) {
        m_name = n;
    }
    QString name() const {
        return m_name;
    }

protected:
    QString m_title;
    QString m_name;
    QString m_query;
    QString m_script;
    QString m_interpreter;
    bool m_externalData;

    ReportPageOptions page;

    KRSectionData * m_pageHeaderFirst;
    KRSectionData * m_pageHeaderOdd;
    KRSectionData * m_pageHeaderEven;
    KRSectionData * m_pageHeaderLast;
    KRSectionData * m_pageHeaderAny;

    KRSectionData * m_reportHeader;
    KRSectionData * m_reportFooter;

    KRSectionData * m_pageFooterFirst;
    KRSectionData * m_pageFooterOdd;
    KRSectionData * m_pageFooterEven;
    KRSectionData * m_pageFooterLast;
    KRSectionData * m_pageFooterAny;

    KRDetailSectionData* m_detailSection;
private:
    bool m_valid;
    void init();

    friend class KoReportPreRendererPrivate;
    friend class KoReportPreRenderer;
    friend class KRScriptHandler;
    friend class Scripting::Report;
};

#endif
