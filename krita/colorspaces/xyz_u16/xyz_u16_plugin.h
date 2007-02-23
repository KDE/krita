/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
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

#ifndef XYZ_U16_PLUGIN_H_
#define XYZ_U16_PLUGIN_H_

#include <QObject>

/**
 * A plugin wrapper around the XYZ U16 color space strategy.
 */
class XYZU16Plugin : public QObject
{
    Q_OBJECT
public:
    XYZU16Plugin(QObject *parent, const QStringList &);
    virtual ~XYZU16Plugin();

};


#endif // XYZ_U16_PLUGIN_H_
