/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dockwidgetfactorybase.h"
#include <QDebug>
DockWidgetFactoryBase::DockWidgetFactoryBase(const QString& _id, KoDockFactoryBase::DockPosition _dockPosition, bool _isCollapsable, bool _defaultCollapsed)
    : m_id(_id),
    m_dockPosition(_dockPosition),
    m_isCollapsable(_isCollapsable),
    m_defaultCollapsed(_defaultCollapsed)
{

}

DockWidgetFactoryBase::~DockWidgetFactoryBase()
{
qDebug() << "Iamdying";
}

bool DockWidgetFactoryBase::defaultCollapsed() const
{
    return m_defaultCollapsed;
}

KoDockFactoryBase::DockPosition DockWidgetFactoryBase::defaultDockPosition() const
{
    return m_dockPosition;
}

QString DockWidgetFactoryBase::id() const
{
    return m_id;
}

bool DockWidgetFactoryBase::isCollapsable() const
{
    return m_isCollapsable;
}
