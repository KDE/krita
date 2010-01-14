/*
 * glsl.h -- Part of Krita
 *
 * Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef GLSLIMAGE_H
#define GLSLIMAGE_H

#include <QVariant>

#include <kparts/plugin.h>

#include "kis_types.h"
class KisView2;


class Glsl : public KParts::Plugin
{
    Q_OBJECT
public:

    Glsl(QObject *parent, const QVariantList &);
    virtual ~Glsl();

private slots:

    void slotActivate();

private:
    KisView2 * m_view;
    KisImageWSP m_image;

};

#endif // GLSLIMAGE_H
