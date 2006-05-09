/*
 *  Copyright (c) 2003 Boudewijn Rempt (boud@valdyas.org)
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef LMS_F32_PLUGIN_H_
#define LMS_F32_PLUGIN_H_

#include <kparts/plugin.h>

/**
 * A plugin wrapper around the LMS F32 colour space strategy.
 */
class LMSF32Plugin : public KParts::Plugin
{
    Q_OBJECT
public:
    LMSF32Plugin(QObject *parent, const QStringList &);
    virtual ~LMSF32Plugin();

};


#endif // LMS_F32_PLUGIN_H_
