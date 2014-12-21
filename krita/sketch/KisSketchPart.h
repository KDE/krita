/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISSKETCHPART_H
#define KISSKETCHPART_H

#include <KisPart.h>

// This class exists primarily to stop KisPart from removing the progress proxy
// we set on the document when saving...
class KisSketchPart : public KisPart
{
public:
    KisSketchPart(QObject *parent = 0);
    virtual ~KisSketchPart();

    virtual bool openFile();
    virtual bool saveFile();
};

#endif // KISSKETCHPART_H
