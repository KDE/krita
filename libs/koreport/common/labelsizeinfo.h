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

#ifndef __LABELSIZEINFO_H__
#define __LABELSIZEINFO_H__

#include <qstring.h>
#include <qstringlist.h>

class LabelSizeInfo
{
public:
    LabelSizeInfo(const QString&, const QString&, int, int, int, int, int, int, int, int);
    LabelSizeInfo();
    virtual ~LabelSizeInfo();

    QString name() const;
    QString paper() const;

    int columns() const;
    int rows() const;

    int width() const;
    int height() const;

    int startX() const;
    int startY() const;

    int xGap() const;
    int yGap() const;

    bool isNull() const;

    static LabelSizeInfo find(const QString &);
    static QStringList labelNames();

protected:
    QString m_name;
    QString m_paper;

    int m_columns;
    int m_rows;

    int m_width;
    int m_height;

    int m_startx;
    int m_starty;

    int m_xgap;
    int m_ygap;

    bool m_null;
};

#endif
