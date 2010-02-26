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

#ifndef __REPORTPAGEOPTIONS_H__
#define __REPORTPAGEOPTIONS_H__

#include <qobject.h>
#include <qstring.h>
#include "koreport_export.h"

class KOREPORT_EXPORT ReportPageOptions : public QObject
{
    Q_OBJECT
public:
    ReportPageOptions();
    ReportPageOptions(const ReportPageOptions &);

    ReportPageOptions & operator=(const ReportPageOptions &);

    enum PageOrientation {
        Landscape = 0, // essentially false
        Portrait = 1   // and true
    };

    qreal getMarginTop();
    void setMarginTop(qreal v);
    qreal getMarginBottom();
    void setMarginBottom(qreal v);
    qreal getMarginLeft();
    void setMarginLeft(qreal v);
    qreal getMarginRight();
    void setMarginRight(qreal v);

    qreal widthPx();
    qreal heightPx();

    const QString & getPageSize();
    void setPageSize(const QString & s);
    qreal getCustomWidth();
    void setCustomWidth(qreal v);
    qreal getCustomHeight();
    void setCustomHeight(qreal v);

    PageOrientation getOrientation();
    bool isPortrait();
    void setOrientation(PageOrientation o);
    void setPortrait(bool yes);

    void setLabelType(const QString &);
    const QString & getLabelType();

signals:
    void pageOptionsChanged();

private:
    qreal m_marginTop;
    qreal m_marginBottom;
    qreal m_marginLeft;
    qreal m_marginRight;

    QString m_pageSize;
    qreal m_customWidth;
    qreal m_customHeight;

    PageOrientation m_orientation;

    QString m_labelType;
};

#endif
