/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DockWidgetFactoryBase.h"
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
