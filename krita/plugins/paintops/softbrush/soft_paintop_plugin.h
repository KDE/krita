/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef SOFT_PAINTOP_PLUGIN_H_
#define SOFT_PAINTOP_PLUGIN_H_

#include <QVariant>

#include <kparts/plugin.h>

/**
 * A plugin wrapper that adds the paintop factories to the paintop registry.
 */
class SoftPaintOpPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    SoftPaintOpPlugin(QObject *parent, const QVariantList &);
    virtual ~SoftPaintOpPlugin();
};

#endif // SOFT_PAINTOP_PLUGIN_H_
