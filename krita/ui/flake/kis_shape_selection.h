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
#ifndef KIS_SHAPE_SELECTION_H
#define KIS_SHAPE_SELECTION_H

#include <KoShapeLayer.h>
#include <KoShapeFactoryBase.h>

#include <kis_selection_component.h>
#include <kis_types.h>

#include <krita_export.h>

class KoStore;
class KisShapeSelectionCanvas;

/**
 *
 */
class KRITAUI_EXPORT KisShapeSelection : public KoShapeLayer, public KisSelectionComponent
{
public:

    KisShapeSelection(KisImageWSP image, KisSelectionSP selection);

    virtual ~KisShapeSelection();

    KisShapeSelection(const KisShapeSelection& rhs);

    KisSelectionComponent* clone();

    bool saveSelection(KoStore * store) const;

    bool loadSelection(KoStore * store);
    /**
     * Renders the shapes to a selection. This method should only be called
     * by KisSelection to update it's projection.
     *
     * @param projection the target selection
     */
    virtual void renderToProjection(KisSelection* projection);
    virtual void renderToProjection(KisSelection* projection, const QRect& r);

    virtual void setDirty();

    void addChild(KoShape *object);
    void removeChild(KoShape *object);

    virtual QPainterPath selectionOutline();

    KoShapeManager *shapeManager() const;

protected:

    virtual void paintComponent(QPainter& painter, const KoViewConverter& converter);

private:

    void renderSelection(KisSelection* projection, const QRect& r);

    KisImageWSP m_image;
    QPainterPath m_outline;
    bool m_dirty;
    KisShapeSelectionCanvas* m_canvas;

    friend class KisShapeSelectionModel;
};


class KRITAUI_EXPORT KisShapeSelectionFactory : public KoShapeFactoryBase
{
    Q_OBJECT
public:

    KisShapeSelectionFactory(QObject* parent);
    ~KisShapeSelectionFactory() {}

    virtual KoShape *createDefaultShape(KoResourceManager *documentResources = 0) const {
        Q_UNUSED(documentResources);
        return 0;
    }
};

#endif
