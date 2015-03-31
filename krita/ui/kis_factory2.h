/*
 *  kis_factory2.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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

#ifndef KIS_FACTORY_2_H_
#define KIS_FACTORY_2_H_

#include <QStringList>

#include <krita_export.h>

#include <kcomponentdata.h>
#include <kaboutdata.h>

class KRITAUI_EXPORT KisFactory
{

public:
    KisFactory();
    ~KisFactory();

    static K4AboutData *aboutData();
    static const KComponentData &componentData();
    static const QString componentName();

private:
    static KComponentData *s_componentData;
    static K4AboutData *s_aboutData;
};

#endif
