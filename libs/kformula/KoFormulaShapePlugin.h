/* This file is part of the KDE project
 * Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#ifndef KOFORMULASHAPEPLUGIN_H
#define KOFORMULASHAPEPLUGIN_H

#include <QObject>

/**
 * @short A plugin for the formula shape and tool
 *
 * This class implements a formula plugin that is loadable by any flake supporting
 * KOffice application. It only contains a destructor and a constructor where the
 * latter has code in it. All functionality has to be in the constructor.
 * In the constructor the plugin registers a shape and a tool in the KoShapeRegistry
 * so that the applications "know" that the formula plugin exists.
 */
class KoFormulaShapePlugin : public QObject {
    Q_OBJECT

public:
    KoFormulaShapePlugin( QObject* parent, const QStringList& );
    ~KoFormulaShapePlugin();
};

#endif

