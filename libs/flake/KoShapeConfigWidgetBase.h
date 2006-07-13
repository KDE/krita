/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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

#ifndef KOSHAPECONFIGWIDGETBASE_H
#define KOSHAPECONFIGWIDGETBASE_H

#include <QWidget>

#include <koffice_export.h>
#include <kaction.h>

#include <KoUnit.h>

class KoShape;

class FLAKE_EXPORT KoShapeConfigWidgetBase : public QWidget {
public:
    KoShapeConfigWidgetBase() {};
    virtual ~KoShapeConfigWidgetBase() {}

    virtual void open(KoShape *shape) = 0;
    virtual void save() = 0;
    virtual KAction *createAction() = 0;

    virtual void setUnit(KoUnit::Unit unit) { Q_UNUSED(unit); }
};


#endif
