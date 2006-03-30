/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_label_cursor_pos.h"
#include "kis_label_cursor_pos.moc"

KisLabelCursorPos::KisLabelCursorPos(QWidget *parent, const char *name, Qt::WFlags f) : super(parent, name, f)
{
    setText("0:0");
    m_doUpdates = true;
    
    //setMinimumSize( 200, parent->height() - 4);
}

KisLabelCursorPos::~KisLabelCursorPos()
{
}

void KisLabelCursorPos::updatePos(qint32 xpos, qint32 ypos)
{
    if (m_doUpdates) {
        QString s;

        s.sprintf("%d:%d", xpos, ypos);
        setText(s);
    }
}

void KisLabelCursorPos::enter()
{
    m_doUpdates = true;
}

void KisLabelCursorPos::leave()
{
    m_doUpdates = false;
    setText(QString::null);
}

