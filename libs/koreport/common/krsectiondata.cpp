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


#include "krsectiondata.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <QColor>

#include "krlinedata.h"
#include "krfielddata.h"
#include "krtextdata.h"
#include "krbarcodedata.h"
#include "krimagedata.h"
#include "krlabeldata.h"
#include "krchartdata.h"
#include "krcheckdata.h"

KRSectionData::KRSectionData()
{
    createProperties();
}

KRSectionData::KRSectionData(const QDomElement & elemSource)
{
    createProperties();
    m_name = elemSource.tagName();
    setObjectName(m_name);

    if (m_name != "report:section") {
        m_valid = false;
        return;
    }

    m_type = sectionTypeFromString(elemSource.attribute("report:section-type"));
    if (m_type == KRSectionData::None) {
        m_valid = false;
        return;
    }
    m_height->setValue(KoUnit::parseValue(elemSource.attribute("svg:height", "2.0cm")));
    
    m_backgroundColor->setValue(QColor(elemSource.attribute("fo:background-color")));

    QDomNodeList section = elemSource.childNodes();
    for (int nodeCounter = 0; nodeCounter < section.count(); nodeCounter++) {
        QDomElement elemThis = section.item(nodeCounter).toElement();

        if (elemThis.tagName() == "report:label") {
            KRLabelData * label = new KRLabelData(elemThis);
            m_objects.append(label);
        } else if (elemThis.tagName() == "report:field") {
            KRFieldData * field = new KRFieldData(elemThis);
            m_objects.append(field);
        } else if (elemThis.tagName() == "report:text") {
            KRTextData * text = new KRTextData(elemThis);
            m_objects.append(text);
        } else if (elemThis.tagName() == "report:line") {
            KRLineData * line = new KRLineData(elemThis);
            m_objects.append(line);
        } else if (elemThis.tagName() == "report:barcode") {
            KRBarcodeData * bc = new KRBarcodeData(elemThis);
            m_objects.append(bc);
        } else if (elemThis.tagName() == "report:image") {
            KRImageData * img = new KRImageData(elemThis);
            m_objects.append(img);
        } else if (elemThis.tagName() == "report:chart") {
            KRChartData * chart = new KRChartData(elemThis);
            m_objects.append(chart);
        } else if (elemThis.tagName() == "report:check") {
            KRCheckData * check = new KRCheckData(elemThis);
            m_objects.append(check);
        } else
            kDebug() << "While parsing section encountered an unknown element: " << elemThis.tagName();
    }
    qSort(m_objects.begin(), m_objects.end(), zLessThan);
    m_valid = true;
}

KRSectionData::~KRSectionData()
{

}

bool KRSectionData::zLessThan(KRObjectData* s1, KRObjectData* s2)
{
    return s1->Z < s2->Z;
}

bool KRSectionData::xLessThan(KRObjectData* s1, KRObjectData* s2)
{
    return s1->position().toPoint().x() < s2->position().toPoint().x();
}

void KRSectionData::createProperties()
{
    m_set = new KoProperty::Set(0, "Section");

    m_height = new KoProperty::Property("height", KoUnit::unit("cm").fromUserValue(2.0), i18n("Height"));
    m_backgroundColor = new KoProperty::Property("background-color", Qt::white, i18n("Background Color"));
    m_height->setOption("unit", "cm");

    m_set->addProperty(m_height);
    m_set->addProperty(m_backgroundColor);
}

QString KRSectionData::name() const
{
    return (m_name + '-' + sectionTypeString(m_type));
}

QString KRSectionData::sectionTypeString(KRSectionData::Section s)
{
    QString sectiontype;
    switch (s) {
    case KRSectionData::PageHeaderAny:
        sectiontype = "header-page-any";
        break;
    case KRSectionData::PageHeaderEven:
        sectiontype = "header-page-even";
        break;
    case KRSectionData::PageHeaderOdd:
        sectiontype = "header-page-odd";
        break;
    case KRSectionData::PageHeaderFirst:
        sectiontype = "header-page-first";
        break;
    case KRSectionData::PageHeaderLast:
        sectiontype = "header-page-last";
        break;
    case KRSectionData::PageFooterAny:
        sectiontype = "footer-page-any";
        break;
    case KRSectionData::PageFooterEven:
        sectiontype = "footer-page-even";
        break;
    case KRSectionData::PageFooterOdd:
        sectiontype = "footer-page-odd";
        break;
    case KRSectionData::PageFooterFirst:
        sectiontype = "footer-page-first";
        break;
    case KRSectionData::PageFooterLast:
        sectiontype = "footer-page-last";
        break;
    case KRSectionData::ReportHeader:
        sectiontype = "header-report";
        break;
    case KRSectionData::ReportFooter:
        sectiontype = "footer-report";
        break;
    case KRSectionData::GroupHeader:
        sectiontype = "group-header";
        break;
    case KRSectionData::GroupFooter:
        sectiontype = "group-footer";
        break;
    case KRSectionData::Detail:
        sectiontype = "detail";
        break;
    }

    return sectiontype;
}

KRSectionData::Section KRSectionData::sectionTypeFromString(const QString& s)
{
    KRSectionData::Section sec;
    kDebug() << "Determining section type for " << s;

    if (s == "header-page-any")
        sec = KRSectionData::PageHeaderAny;
    else if (s == "header-page-even")
        sec = KRSectionData::PageHeaderEven;
    else if (s == "header-page-odd")
        sec = KRSectionData::PageHeaderOdd;
    else if (s == "header-page-first")
        sec = KRSectionData::PageHeaderFirst;
    else if (s == "header-page-last")
        sec = KRSectionData::PageHeaderLast;
    else if (s == "header-report")
        sec = KRSectionData::ReportHeader;
    else if (s == "footer-page-any")
        sec = KRSectionData::PageFooterAny;
    else if (s == "footer-page-even")
        sec = KRSectionData::PageFooterEven;
    else if (s == "footer-page-odd")
        sec = KRSectionData::PageFooterOdd;
    else if (s == "footer-page-first")
        sec = KRSectionData::PageFooterFirst;
    else if (s == "footer-page-last")
        sec = KRSectionData::PageFooterLast;
    else if (s == "footer-report")
        sec = KRSectionData::ReportFooter;
    else if (s == "group-header")
        sec = KRSectionData::GroupHeader;
    else if (s == "group-footer")
        sec = KRSectionData::GroupFooter;
    else if (s == "detail")
        sec = KRSectionData::Detail;
    else
        sec = KRSectionData::None;

    return sec;
}
