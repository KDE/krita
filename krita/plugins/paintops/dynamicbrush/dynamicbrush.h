/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef DYNAMIC_BRUSH_H
#define DYNAMIC_BRUSH_H

#include <kparts/plugin.h>

/**
 * A plugin wrapper that adds the Dynamic brush paintop
 */
class DynamicBrush : public KParts::Plugin
{
    Q_OBJECT
    public:
        DynamicBrush(QObject *parent, const QStringList &);
        virtual ~DynamicBrush();
};

#endif // DYNAMIC_BRUSH_H
