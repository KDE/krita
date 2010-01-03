/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <kparts/plugin.h>
#include "filter/kis_color_transformation_filter.h"

class KritaExample : public QObject
{
public:
    KritaExample(QObject *parent, const QStringList &);
    virtual ~KritaExample();
};

class KisFilterInvert : public KisColorTransformationFilter
{
public:
    KisFilterInvert();
public:

    virtual KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const;

    static inline KoID id() {
        return KoID("invert", i18n("Invert"));
    }

};

#endif
