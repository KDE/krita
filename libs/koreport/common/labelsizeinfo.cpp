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

#include "labelsizeinfo.h"

static LabelSizeInfo s_labels[] = {
//"LABEL_NAME","Paper Size","COLUMNS","ROWS","WIDTH","HEIGHT","STARTXOFFSET","STARTYOFFSET","HORIZONTALGAP","VERTICALGAP"
    LabelSizeInfo("Avery 5263", "Letter", 2, 5, 400, 200, 25, 50, 0, 0),
    LabelSizeInfo("Avery 5264", "Letter", 2, 3, 400, 333, 25, 75, 0, 0),
    LabelSizeInfo("Avery 8460", "Letter", 3, 10, 262, 100, 32, 50, 0, 0),
    LabelSizeInfo("CILS ALP1-9200-1", "Letter", 3, 7, 200, 100, 62, 62, 81, 50),
    LabelSizeInfo()               // Null Label Size
};

LabelSizeInfo LabelSizeInfo::find(const QString & name)
{
    int i = 0;
    while (!s_labels[i].isNull() && s_labels[i].m_name != name)
        i++;
    return s_labels[i];
}

QStringList LabelSizeInfo::labelNames()
{
    QStringList l;
    for (int i = 0; !s_labels[i].isNull(); i++)
        l.append(s_labels[i].m_name);
    return l;
}

LabelSizeInfo::LabelSizeInfo(const QString & n, const QString & p, int c,
                             int r, int w, int h, int sx, int sy, int xg,
                             int yg)
{
    m_name = n;
    m_paper = p;

    m_columns = c;
    m_rows = r;

    m_width = w;
    m_height = h;

    m_startx = sx;
    m_starty = sy;

    m_xgap = xg;
    m_ygap = yg;

    m_null = false;
}

LabelSizeInfo::LabelSizeInfo()
{
    m_columns = 0;
    m_rows = 0;

    m_width = 0;
    m_height = 0;

    m_startx = 0;
    m_starty = 0;

    m_xgap = 0;
    m_ygap = 0;

    m_null = true;
}

LabelSizeInfo::~LabelSizeInfo()
{
}

QString LabelSizeInfo::name() const
{
    return m_name;
}

QString LabelSizeInfo::paper() const
{
    return m_paper;
}

int LabelSizeInfo::columns() const
{
    return m_columns;
}
int LabelSizeInfo::rows() const
{
    return m_rows;
}

int LabelSizeInfo::width() const
{
    return m_width;
}

int LabelSizeInfo::height() const
{
    return m_height;
}

int LabelSizeInfo::startX() const
{
    return m_startx;
}

int LabelSizeInfo::startY() const
{
    return m_starty;
}

int LabelSizeInfo::xGap() const
{
    return m_xgap;
}

int LabelSizeInfo::yGap() const
{
    return m_ygap;
}

bool LabelSizeInfo::isNull() const
{
    return m_null;
}
