/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tool_button.h"

#include <QMouseEvent>
#include <QStyleOptionToolButton>

KisToolButton::KisToolButton(QWidget *parent) :
    QToolButton(parent)
{
    m_tabletContact = false;
}

void KisToolButton::mousePressEvent(QMouseEvent *e)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QRect popupr = style()->subControlRect(QStyle::CC_ToolButton, &opt,
                                           QStyle::SC_ToolButtonMenu, this);
    if (popupr.isValid() && !popupr.contains(e->pos())) {
        QToolButton::mousePressEvent(e);
    } else {
        m_tabletContact = true;
    }
}

void KisToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_tabletContact) {
        QToolButton::mousePressEvent(e);
    } else {
        QToolButton::mouseReleaseEvent(e);
    }
    m_tabletContact = false;
}
