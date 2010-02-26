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
#ifndef KRDETAILSECTIONDATA_H
#define KRDETAILSECTIONDATA_H

#include <QObject>
#include <qdom.h>
#include "KoReportData.h"

class KRSectionData;
class ORDetailGroupSectionData;
/**
 @author Adam Pigg <adam@piggz.co.uk>
*/
class KRDetailSectionData : public QObject
{
    Q_OBJECT
public:
    KRDetailSectionData();
    KRDetailSectionData(const QDomElement &);
    ~KRDetailSectionData();

    enum PageBreak {
        BreakNone = 0,
        BreakAtEnd = 1
    };

    QString m_name;
    int m_pageBreak;
    QList<KoReportData::SortedField> m_sortedFields;

    KRSectionData * m_detailSection;

    QList<ORDetailGroupSectionData*> m_groupList;

    bool isValid() {
        return m_valid;
    }

private:
    bool m_valid;
};

class ORDetailGroupSectionData
{
public:
    ORDetailGroupSectionData();

    enum PageBreak {
        BreakNone = 0,
        BreakAfterGroupFooter = 1,
        BreakBeforeGroupHeader = 2
    };

    //QString name;
    QString m_column;
    PageBreak m_pagebreak;
    Qt::SortOrder m_sort;

    KRSectionData *m_groupHeader;
    KRSectionData *m_groupFooter;
};

#endif
