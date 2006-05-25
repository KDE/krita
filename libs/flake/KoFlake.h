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

/**
 * \mainpage
 * The Flake library is a low level library for all kinds of graphical content
 * to be placed on any KOffice canvas. This includes a line to text-areas or
 * even movies.  Just as important, this library will include tools to manipulate
 * the graphical content. At least at the level of Flake objects.  This goes from
 * moving/rotating the object to a basis for complex brushes for a paint program.
 * <p>Use KoShape as a base object for any application-specific graphical
 * content, and extend KoShapeContainer for objects that can contain others.
 * <p>KoShape is the base class for all flake objects. Flake objects extend it
 * to allow themselves to be manipulated by the KoTool s. The content of such an
 * object is independent and based on the model of the data this object represents.
 */

/**
 * Flake reference
 */
class KoFlake {
public:
    /// the selection type for KoSelection::selectedObjects()
    enum SelectionType {
        FullSelection,
        StrippedSelection
    };

    /// Enum determining which handle is meant, used in KoInteractionTool
    enum SelectionHandle {
        TopMiddleHandle,
        TopRightHandle,
        RightMiddleHandle,
        BottomRightHandle,
        BottomMiddleHandle,
        BottomLeftHandle,
        LeftMiddleHandle,
        TopLeftHandle,
        NoHandle
    };

private:
    KoFlake();
};
