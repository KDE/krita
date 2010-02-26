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

#include "krdetailsectiondata.h"
#include "krsectiondata.h"

#include <kdebug.h>

KRDetailSectionData::KRDetailSectionData()
{
    m_pageBreak = BreakNone;
    m_detailSection = 0;
}

KRDetailSectionData::KRDetailSectionData(const QDomElement &elemSource)
{
    m_valid = false;
    kDebug() << elemSource.tagName();
    if (elemSource.tagName() != "report:detail") {
        return;
    }

    bool have_detail = false;

    QDomNodeList sections = elemSource.childNodes();

    for (int nodeCounter = 0; nodeCounter < sections.count(); nodeCounter++) {
        QDomElement elemThis = sections.item(nodeCounter).toElement();

        if (elemThis.tagName() == "report:group") {
            ORDetailGroupSectionData * dgsd = new ORDetailGroupSectionData();

            if ( elemThis.hasAttribute( "report:group-column" ) ) {
                dgsd->m_column = elemThis.attribute( "report:group-column" );
            }

            if ( elemThis.hasAttribute( "report:group-page-break" ) ) {
                QString s = elemThis.attribute( "report:group-page-break" );
                if ( s == "after-footer" ) {
                    dgsd->m_pagebreak = ORDetailGroupSectionData::BreakAfterGroupFooter;
                } else if ( s == "before-header" ) {
                    dgsd->m_pagebreak = ORDetailGroupSectionData::BreakBeforeGroupHeader;
                } else {
                    dgsd->m_pagebreak = ORDetailGroupSectionData::BreakNone;
                }
            }

            if (elemThis.attribute("report:group-sort", "ascending") == "ascending") {
                dgsd->m_sort = Qt::AscendingOrder;
            } else {
                dgsd->m_sort = Qt::DescendingOrder;
            }
            
            for ( QDomElement e = elemThis.firstChildElement( "report:section" ); ! e.isNull(); e = e.nextSiblingElement( "report:section" ) ) {
                QString s = e.attribute( "report:section-type" );
                if ( s == "group-header" ) {
                    KRSectionData * sd = new KRSectionData(e);
                    if (sd->isValid()) {
                        dgsd->m_groupHeader = sd;
                    } else {
                        delete sd;
                    }
                } else if ( s == "group-footer" ) {
                    KRSectionData * sd = new KRSectionData(e);
                    if (sd->isValid()) {
                        dgsd->m_groupFooter = sd;
                    } else {
                        delete sd;
                    }
                }
            }
            m_groupList.append(dgsd);
            KoReportData::SortedField s;
            s.field = dgsd->m_column;
            s.order = dgsd->m_sort;
            m_sortedFields.append(s);
	    
        } else if (elemThis.tagName() == "report:section" && elemThis.attribute("report:section-type") == "detail") {
            KRSectionData * sd = new KRSectionData(elemThis);
            if (sd->isValid()) {
                m_detailSection = sd;
                have_detail = true;
            } else
                delete sd;
        } else {
            kDebug() << "While parsing detail section encountered an unknown element: " << elemThis.tagName();
        }
    }
    
    m_valid = true;
}

KRDetailSectionData::~KRDetailSectionData()
{
}

ORDetailGroupSectionData::ORDetailGroupSectionData()
{
    m_pagebreak = BreakNone;
    m_sort = Qt::AscendingOrder;
    m_groupHeader = 0;
    m_groupFooter = 0;
}

