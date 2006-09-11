/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOINSETS_H
#define KOINSETS_H

#include <koffice_export.h>

/**
 * An Insets object is a representation of the borders of a shape.
 */
struct FLAKE_EXPORT KoInsets {
public:
    KoInsets()
    {
    }

    /**
     * Constructor.
     * @param top the inset at the top.
     * @param left the inset at the left
     * @param bottom the inset at the bottom
     * @param right the inset at the right
     */
    KoInsets(double top, double left, double bottom, double right) {
        this->top = top;
        this->left = left;
        this->bottom = bottom;
        this->right = right;
    }
    double top;     ///< Top inset
    double bottom;  ///< Bottom inset
    double left;    ///< Left inset
    double right;   ///< Right inset
};

#endif
