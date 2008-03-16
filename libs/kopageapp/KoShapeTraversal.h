/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPETRAVERSAL_H
#define KOSHAPETRAVERSAL_H

class KoShape;
class KoShapeContainer;
class QString;

/**
 * @brief Class to traverse shapes
 */
class KoShapeTraversal
{
public:
    /**
     * @brief Get the next shape in the tree
     *
     * @param The current shape
     *
     * @return The shape after the given one or 0 if there is no next shape
     */
    static KoShape * nextShape( const KoShape * current );

    /**
     * @brief Get the next shape in the tree of the given type
     *
     * @param current The current shape
     * @param shapeId The shape id of the shape to find
     *
     * @return The shape with the shape id given after the current one or 0 if there is no 
     *         such shape after the current one
     */
    static KoShape * nextShape( const KoShape * current, const QString & shapeId );

    /**
     * @brief Get the previous shape in the tree
     *
     * @param The current shape
     *
     * @return The shape before the given one
     */
    static KoShape * previousShape( const KoShape * current );

    /**
     * @brief Get the previous shape in the tree of the given type
     *
     * @param current The current shape
     * @param shapeId The shape id of the shape to find
     *
     * @return The shape with the shape id given before the current one or 0 if there is no 
     *         such shape before the current one
     */
    static KoShape * previousShape( const KoShape * current, const QString & shapeId );

    /**
     * @brief Get the last shape in subtree
     *
     * @param current The current shape
     *
     * @return The last shape in the current sub tree
     */
    static KoShape * last( KoShape * current );

private:
    static KoShape * nextShapeStep( const KoShape * current, const KoShapeContainer * parent );

    static KoShape * previousShapeStep( const KoShape * current, const KoShapeContainer * parent );
};

#endif /* KOSHAPETRAVERSAL_H */
