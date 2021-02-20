/*
 *  SPDX-FileCopyrightText: 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "DockWidgetFactoryBase.h"
#include <QDebug>

DockWidgetFactoryBase::DockWidgetFactoryBase(const QString& _id, KoDockFactoryBase::DockPosition _dockPosition)
    : m_id(_id),
    m_dockPosition(_dockPosition)
{

}

DockWidgetFactoryBase::~DockWidgetFactoryBase()
{
}

KoDockFactoryBase::DockPosition DockWidgetFactoryBase::defaultDockPosition() const
{
    return m_dockPosition;
}

QString DockWidgetFactoryBase::id() const
{
    return m_id;
}
