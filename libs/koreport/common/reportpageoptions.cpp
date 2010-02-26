/*
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

#include "reportpageoptions.h"
#include <KoPageFormat.h>
#include <KoUnit.h>
#include <KoDpi.h>

ReportPageOptions::ReportPageOptions()
        : QObject(), m_pageSize("Letter")
{
    m_marginTop = m_marginBottom = 1.0;
    m_marginLeft = m_marginRight = 1.0;

    m_orientation = Portrait;

    m_customWidth = 8.5;
    m_customHeight = 11.0;
}

ReportPageOptions::ReportPageOptions(const ReportPageOptions & rpo)
        : QObject()
{
    m_marginTop = rpo.m_marginTop;
    m_marginBottom = rpo.m_marginBottom;
    m_marginLeft = rpo.m_marginLeft;
    m_marginRight = rpo.m_marginRight;

    m_pageSize = rpo.m_pageSize;
    m_customWidth = rpo.m_customWidth;
    m_customHeight = rpo.m_customHeight;

    m_orientation = rpo.m_orientation;

    m_labelType = rpo.m_labelType;
}

ReportPageOptions & ReportPageOptions::operator=(const ReportPageOptions & rpo)
{
    m_marginTop = rpo.m_marginTop;
    m_marginBottom = rpo.m_marginBottom;
    m_marginLeft = rpo.m_marginLeft;
    m_marginRight = rpo.m_marginRight;

    m_pageSize = rpo.m_pageSize;
    m_customWidth = rpo.m_customWidth;
    m_customHeight = rpo.m_customHeight;

    m_orientation = rpo.m_orientation;

    m_labelType = rpo.m_labelType;

    return *this;
}

qreal ReportPageOptions::getMarginTop()
{
    return m_marginTop;
}

void ReportPageOptions::setMarginTop(qreal v)
{
    if (m_marginTop == v)
        return;

    m_marginTop = v;
    emit pageOptionsChanged();
}

qreal ReportPageOptions::getMarginBottom()
{
    return m_marginBottom;
}

void ReportPageOptions::setMarginBottom(qreal v)
{
    if (m_marginBottom == v)
        return;

    m_marginBottom = v;
    emit pageOptionsChanged();
}

qreal ReportPageOptions::getMarginLeft()
{
    return m_marginLeft;
}

void ReportPageOptions::setMarginLeft(qreal v)
{
    if (m_marginLeft == v)
        return;

    m_marginLeft = v;
    emit pageOptionsChanged();
}

qreal ReportPageOptions::getMarginRight()
{
    return m_marginRight;
}

void ReportPageOptions::setMarginRight(qreal v)
{
    if (m_marginRight == v)
        return;

    m_marginRight = v;
    emit pageOptionsChanged();
}

const QString & ReportPageOptions::getPageSize()
{
    return m_pageSize;
}
void ReportPageOptions::setPageSize(const QString & s)
{
    if (m_pageSize == s)
        return;

    m_pageSize = s;
    emit pageOptionsChanged();
}
qreal ReportPageOptions::getCustomWidth()
{
    return m_customWidth;
}
void ReportPageOptions::setCustomWidth(qreal v)
{
    if (m_customWidth == v)
        return;

    m_customWidth = v;
    emit pageOptionsChanged();
}
qreal ReportPageOptions::getCustomHeight()
{
    return m_customHeight;
}
void ReportPageOptions::setCustomHeight(qreal v)
{
    if (m_customHeight == v)
        return;

    m_customHeight = v;
    emit pageOptionsChanged();
}

ReportPageOptions::PageOrientation ReportPageOptions::getOrientation()
{
    return m_orientation;
}

bool ReportPageOptions::isPortrait()
{
    return (m_orientation == Portrait);
}

void ReportPageOptions::setOrientation(PageOrientation o)
{
    if (m_orientation == o)
        return;

    m_orientation = o;
    emit pageOptionsChanged();
}
void ReportPageOptions::setPortrait(bool yes)
{
    setOrientation((yes ? Portrait : Landscape));
}

const QString & ReportPageOptions::getLabelType()
{
    return m_labelType;
}
void ReportPageOptions::setLabelType(const QString & type)
{
    if (m_labelType == type)
        return;

    m_labelType = type;
    emit pageOptionsChanged();
}

//Convenience functions that return the page width/height in pixels based on the DPI
qreal ReportPageOptions::widthPx()
{
    int pageWidth;

    if (isPortrait()) {
        pageWidth = KoPageFormat::width(KoPageFormat::formatFromString(getPageSize()), KoPageFormat::Portrait);
    } else {
        pageWidth = KoPageFormat::width(KoPageFormat::formatFromString(getPageSize()), KoPageFormat::Landscape);
    }

    KoUnit pageUnit(KoUnit::Millimeter);
    pageWidth = KoUnit::toInch(pageUnit.fromUserValue(pageWidth)) * KoDpi::dpiX();

    return pageWidth;
}

qreal ReportPageOptions::heightPx()
{
    int pageHeight;

    if (isPortrait()) {
        pageHeight = KoPageFormat::height(KoPageFormat::formatFromString(getPageSize()), KoPageFormat::Portrait);
    } else {
        pageHeight = KoPageFormat::height(KoPageFormat::formatFromString(getPageSize()), KoPageFormat::Landscape);
    }

    KoUnit pageUnit(KoUnit::Millimeter);
    pageHeight = KoUnit::toInch(pageUnit.fromUserValue(pageHeight)) * KoDpi::dpiY();

    return pageHeight;
}

