/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef _KO_SHAPE_CONFIG_FACTORY_
#define _KO_SHAPE_CONFIG_FACTORY_

#include <koffice_export.h>

#include <QString>

class KoShape;
class KoShapeConfigWidgetBase;

class FLAKE_EXPORT KoShapeConfigFactory {
public:
    KoShapeConfigFactory() {}
    virtual ~KoShapeConfigFactory() {}

    virtual KoShapeConfigWidgetBase *createConfigWidget(KoShape *shape) = 0;
    virtual QString name() const = 0;

    virtual int sortingOrder() const { return 1; }
    virtual bool showForShapeId(const QString &id) const { Q_UNUSED(id); return true; }

    static bool compare(KoShapeConfigFactory *f1, KoShapeConfigFactory *f2) {
        return f1->sortingOrder() - f2->sortingOrder() > 0;
    }
};

#endif
