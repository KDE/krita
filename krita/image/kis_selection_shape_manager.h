/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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
#ifndef KIS_SELECTION_SHAPE_MANAGER_H
#define KIS_SELECTION_SHAPE_MANAGER_H

#include <KoShapeManager.h>

/**
 * The KisSelectionShapeManager holds a list of shapes inside a KisSelection.
 *
 */
class KisSelectionShapeManager : public KoShapeManager {

    typedef KoShapeManager super;

public:

    KisSelectionShapeManager(KoCanvasBase *canvas);
    virtual ~KisSelectionShapeManager();

   /**
     * Paint all shapes and the marching ants
     * @param painter the painter to paint into.
     * @param converter to be used
     * @param offset the offset of the marching ants
     */
    virtual void paintShapeSelection( QPainter &painter, const KoViewConverter &converter, int offset );

};

#endif
