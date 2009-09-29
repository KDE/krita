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

#ifndef KIS_SHAPE_SELECTION_MODEL_H
#define KIS_SHAPE_SELECTION_MODEL_H

#include "KoShapeContainerModel.h"
#include "kis_types.h"

class KisShapeSelection;

/**
 *
 */
class KisShapeSelectionModel: public KoShapeContainerModel
{
public:
    KisShapeSelectionModel(KisImageWSP image, KisSelectionSP selection, KisShapeSelection* shapeSelection);
    ~KisShapeSelectionModel();

    void add(KoShape *child);
    void remove(KoShape *child);

    void setClipping(const KoShape *child, bool clipping);
    bool childClipped(const KoShape *child) const;

    int count() const;
    QList<KoShape*> childShapes() const;

    void containerChanged(KoShapeContainer *);
    void childChanged(KoShape * child, KoShape::ChangeType type);
    bool isChildLocked(const KoShape *child) const;

private:
    QMap<KoShape*, QRectF> m_shapeMap;
    KisImageWSP m_image;
    KisSelectionSP m_parentSelection;
    KisShapeSelection* m_shapeSelection;
};

#endif
