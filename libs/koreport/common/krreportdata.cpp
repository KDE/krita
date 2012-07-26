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
#include "krreportdata.h"
#include <kdebug.h>
#include <KoUnit.h>
#include <KoDpi.h>
#include "krdetailsectiondata.h"
#include "KoReportItemBase.h"

void KoReportReportData::init()
{
    m_pageHeaderFirst = m_pageHeaderOdd = m_pageHeaderEven = m_pageHeaderLast = m_pageHeaderAny = 0;
    m_pageFooterFirst = m_pageFooterOdd = m_pageFooterEven = m_pageFooterLast = m_pageFooterAny = 0;
    m_reportHeader = m_reportFooter = 0;
}

KoReportReportData::KoReportReportData(QObject *parent)
        : QObject(parent)
        , m_detailSection(0)
{
    init();
    m_valid = true;
}

KoReportReportData::KoReportReportData(const QDomElement & elemSource, QObject *parent)
        : QObject(parent)
        , m_detailSection(0)
{
    m_valid = false;
    init();
    //bool valid; //used for local unit conversions

    kDebug();

    if (elemSource.tagName() != "report:content") {
        kDebug() << "QDomElement passed to parseReport() was not <report:content> tag";
        kDebug() << elemSource.text();
        return;
    }

    //qreal d = 0.0;

    QDomNodeList sections = elemSource.childNodes();
    for (int nodeCounter = 0; nodeCounter < sections.count(); nodeCounter++) {
        QDomElement elemThis = sections.item(nodeCounter).toElement();
        if (elemThis.tagName() == "report:title") {
            m_title = elemThis.text();
        } else if (elemThis.tagName() == "report:script") {
            m_script = elemThis.text();
            m_interpreter = elemThis.attribute("report:script-interpreter");
        } else if (elemThis.tagName() == "report:page-style") {
            QString pagetype = elemThis.firstChild().nodeValue();

            if (pagetype == "predefined") {
                page.setPageSize(elemThis.attribute("report:page-size", "A4"));
            } else if (pagetype == "custom") {
                page.setCustomWidth(POINT_TO_INCH(KoUnit::parseValue(elemThis.attribute("report:custom-page-width", ""))) * KoDpi::dpiX());
                page.setCustomHeight(POINT_TO_INCH(KoUnit::parseValue(elemThis.attribute("report:custom-page-height", ""))) * KoDpi::dpiY());
                page.setPageSize("Custom");
            } else if (pagetype == "label") {
                page.setLabelType(elemThis.firstChild().nodeValue());
            }
            //@todo add config for default margins or add within templates support
            page.setMarginBottom(POINT_TO_INCH(KoUnit::parseValue(elemThis.attribute("fo:margin-bottom", "1.0cm"))) * KoDpi::dpiY());
            page.setMarginTop(POINT_TO_INCH(KoUnit::parseValue(elemThis.attribute("fo:margin-top", "1.0cm"))) * KoDpi::dpiY());
            page.setMarginLeft(POINT_TO_INCH(KoUnit::parseValue(elemThis.attribute("fo:margin-left", "1.0cm"))) * KoDpi::dpiX());
            page.setMarginRight(POINT_TO_INCH(KoUnit::parseValue(elemThis.attribute("fo:margin-right", "1.0cm"))) * KoDpi::dpiX());

            page.setPortrait(elemThis.attribute("report:print-orientation", "portrait") == "portrait");

        } else if (elemThis.tagName() == "report:body") {
            QDomNodeList sectionlist = elemThis.childNodes();
            QDomNode sec;

            for (int s = 0; s < sectionlist.count(); ++s) {
                sec = sectionlist.item(s);
                if (sec.isElement()) {
                    QString sn = sec.nodeName().toLower();
                    kDebug() << sn;
                    if (sn == "report:section") {
                        KRSectionData * sd = new KRSectionData(sec.toElement(), this);
                        if (!sd->isValid()) {
                            kDebug() << "Invalid section";
                            delete sd;
                        } else {
                            kDebug() << "Adding section of type " << sd->type();

                            switch (sd->type()) {
                            case KRSectionData::PageHeaderFirst:
                                m_pageHeaderFirst = sd;
                                break;
                            case KRSectionData::PageHeaderOdd:
                                m_pageHeaderOdd = sd;
                                break;
                            case KRSectionData::PageHeaderEven:
                                m_pageHeaderEven = sd;
                                break;
                            case KRSectionData::PageHeaderLast:
                                m_pageHeaderLast = sd;
                                break;
                            case KRSectionData::PageHeaderAny:
                                m_pageHeaderAny = sd;
                                break;
                            case KRSectionData::ReportHeader:
                                m_reportHeader = sd;
                                break;
                            case KRSectionData::ReportFooter:
                                m_reportFooter = sd;
                                break;
                            case KRSectionData::PageFooterFirst:
                                m_pageFooterFirst = sd;
                                break;
                            case KRSectionData::PageFooterOdd:
                                m_pageFooterOdd = sd;
                                break;
                            case KRSectionData::PageFooterEven:
                                m_pageFooterEven = sd;
                                break;
                            case KRSectionData::PageFooterLast:
                                m_pageFooterLast = sd;
                                break;
                            case KRSectionData::PageFooterAny:
                                m_pageFooterAny = sd;
                                break;
                            default:
                                ;
                            }
                        }

                    } else if (sn == "report:detail") {
                        KRDetailSectionData * dsd = new KRDetailSectionData(sec.toElement(), this);

                        if (dsd->isValid()) {
                            m_detailSection = dsd;
                        } else {
                            kDebug() << "Invalid detail section";
                            delete dsd;
                        }
                    }
                } else {
                    kDebug() << "Encountered an unknown Element: "  << elemThis.tagName();
                }
            }
        }

        m_valid = true;
    }
}


KoReportReportData::~KoReportReportData()
{
}

QList<KoReportItemBase*> KoReportReportData::objects() const
{
    QList<KoReportItemBase*> obs;
    KRSectionData *sec;

    for (int i = 1; i <= KRSectionData::PageFooterAny; i++) {
        sec = section((KRSectionData::Section)i);
        if (sec) {
            obs << sec->objects();
        }
    }

    if (m_detailSection) {
        kDebug() << "Number of groups: " << m_detailSection->m_groupList.count();
        foreach(ORDetailGroupSectionData* g, m_detailSection->m_groupList) {
            if (g->m_groupHeader) {
                obs << g->m_groupHeader->objects();
            }
            if (g->m_groupFooter) {
                obs << g->m_groupFooter->objects();
            }
        }
        if (m_detailSection->m_detailSection)
            obs << m_detailSection->m_detailSection->objects();
    }

    kDebug() << "Object List:";
    foreach(KoReportItemBase* o, obs) {
        kDebug() << o->entityName();
    }
    return obs;
}

KoReportItemBase* KoReportReportData::object(const QString& n) const
{
    QList<KoReportItemBase*> obs = objects();

    foreach(KoReportItemBase* o, obs) {
        if (o->entityName() == n) {
            return o;
        }
    }
    return 0;
}

QList<KRSectionData*> KoReportReportData::sections() const
{
    QList<KRSectionData*> secs;
    KRSectionData *sec;
    for (int i = 0; i < 12 ; ++i) {
        sec = section((KRSectionData::Section)(i + 1));
        if (sec) {
            secs << sec;
        }
    }

    if (m_detailSection) {
        kDebug() << "Number of groups: " << m_detailSection->m_groupList.count();
        foreach(ORDetailGroupSectionData* g, m_detailSection->m_groupList) {
            if (g->m_groupHeader) {
                secs << g->m_groupHeader;
            }
            if (g->m_groupFooter) {
                secs << g->m_groupFooter;
            }
        }
        if (m_detailSection->m_detailSection)
            secs << m_detailSection->m_detailSection;
    }

    return secs;
}

KRSectionData* KoReportReportData::section(const QString& sn) const
{
    QList<KRSectionData*> secs = sections();

    foreach(KRSectionData *sec, secs) {
        if (sec->name() == sn) {
            return sec;
        }
    }
    return 0;
}

KRSectionData* KoReportReportData::section(KRSectionData::Section s) const
{
    KRSectionData *sec;
    switch (s) {
    case KRSectionData::PageHeaderAny:
        sec = m_pageHeaderAny;
        break;
    case KRSectionData::PageHeaderEven:
        sec = m_pageHeaderEven;
        break;
    case KRSectionData::PageHeaderOdd:
        sec = m_pageHeaderOdd;
        break;
    case KRSectionData::PageHeaderFirst:
        sec = m_pageHeaderFirst;
        break;
    case KRSectionData::PageHeaderLast:
        sec = m_pageHeaderLast;
        break;
    case KRSectionData::PageFooterAny:
        sec = m_pageFooterAny;
        break;
    case KRSectionData::PageFooterEven:
        sec = m_pageFooterEven;
        break;
    case KRSectionData::PageFooterOdd:
        sec = m_pageFooterOdd;
        break;
    case KRSectionData::PageFooterFirst:
        sec = m_pageFooterFirst;
        break;
    case KRSectionData::PageFooterLast:
        sec = m_pageFooterLast;
        break;
    case KRSectionData::ReportHeader:
        sec = m_reportHeader;
        break;
    case KRSectionData::ReportFooter:
        sec = m_reportFooter;
        break;
    default:
        sec = 0;
    }
    return sec;
}

ReportPageOptions KoReportReportData::pageOptions() const
{
    return page;
}
