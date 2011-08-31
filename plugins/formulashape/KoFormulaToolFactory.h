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

#ifndef KOFORMULATOOLFACTORY_H
#define KOFORMULATOOLFACTORY_H

#include <KoToolFactoryBase.h>

/**
 * @short The factory for KoFormulaTool
 *
 * This reimplements the KoToolFactoryBase class from the flake library in order
 * to provide a factory for the KoToolBase based class KoFormulaTool. This is the
 * KoToolBase that is used to edit a KoFormulaShape.
 * This class is part of the FormulaShape plugin and follows the factory design
 * pattern.
 */
class KoFormulaToolFactory : public KoToolFactoryBase {
public:
    /// The constructor - reimplemented from KoToolFactoryBase
    explicit KoFormulaToolFactory();

    /// The destructor - reimplemented from KoToolFactoryBase
    ~KoFormulaToolFactory();

    /// @return an instance of KoFormulaTool
    KoToolBase* createTool( KoCanvasBase* canvas );
};

#endif
