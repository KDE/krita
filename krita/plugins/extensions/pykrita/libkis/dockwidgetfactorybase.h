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

#ifndef LIBKIS_DOCKWIDGETFACTORY_H
#define LIBKIS_DOCKWIDGETFACTORY_H

#include <QString>
#include <KoDockFactoryBase.h>

#include <krita_export.h>

class LIBKIS_EXPORT DockWidgetFactoryBase : public KoDockFactoryBase
{
public:
    DockWidgetFactoryBase(const QString& _id, DockPosition _dockPosition, bool _isCollapsable = true, bool _defaultCollapsed = false);
    virtual ~DockWidgetFactoryBase();
    virtual QString id() const;
    virtual DockPosition defaultDockPosition() const;
    virtual bool isCollapsable() const;
    virtual bool defaultCollapsed() const;
private:
    QString m_id;
    DockPosition m_dockPosition;
    bool m_isCollapsable, m_defaultCollapsed;
};

#endif
