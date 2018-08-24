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
#include <KoShapeUserData.h>
#include <KoShapeLoadingContext.h>

#include <kis_selection_component.h>
#include <kis_types.h>

#include <kritaui_export.h>

class KoStore;
class KoShapeManager;
class KisShapeSelectionCanvas;
class KisShapeSelectionModel;
class KisImageViewConverter;
class KUndo2Command;

/**
 * The marker class.
 * It is added to the shape's user data to show this shape
 * is a part of a shape selection
 */
class KisShapeSelectionMarker : public KoShapeUserData
{
    KoShapeUserData* clone() const override {
        return new KisShapeSelectionMarker(*this);
    }
};


class KRITAUI_EXPORT KisShapeSelection : public KoShapeLayer, public KisSelectionComponent
{
    KisShapeSelection(const KisShapeSelection& rhs);
public:

    KisShapeSelection(KoShapeControllerBase *shapeControllerBase, KisImageWSP image, KisSelectionWSP selection);

    ~KisShapeSelection() override;

    KisShapeSelection(const KisShapeSelection& rhs, KisSelection* selection);

    KisSelectionComponent* clone(KisSelection* selection) override;

    bool saveSelection(KoStore * store) const;

    bool loadSelection(KoStore * store);
    /**
     * Renders the shapes to a selection. This method should only be called
     * by KisSelection to update it's projection.
     *
     * @param projection the target selection
     */
    void renderToProjection(KisPaintDeviceSP projection) override;
    void renderToProjection(KisPaintDeviceSP projection, const QRect& r) override;

    KUndo2Command* resetToEmpty() override;
    bool isEmpty() const override;

    QPainterPath outlineCache() const override;
    bool outlineCacheValid() const override;
    void recalculateOutlineCache() override;

    KoShapeManager *shapeManager() const;

    void moveX(qint32 x) override;
    void moveY(qint32 y) override;

    KUndo2Command* transform(const QTransform &transform) override;
protected:

    void paintComponent(QPainter& painter, const KoViewConverter& converter, KoShapePaintingContext &paintcontext) override;

private:
    friend class KisTakeAllShapesCommand;
    void setUpdatesEnabled(bool enabled);
    bool updatesEnabled() const;

private:

    void renderSelection(KisPaintDeviceSP projection, const QRect& r);

    KisImageWSP m_image;
    QPainterPath m_outline;
    KisImageViewConverter *m_converter;
    KisShapeSelectionCanvas *m_canvas;
    KisShapeSelectionModel *m_model;
    KoShapeControllerBase *m_shapeControllerBase;
    friend class KisShapeSelectionModel;
};


class KRITAUI_EXPORT KisShapeSelectionFactory : public KoShapeFactoryBase
{
public:

    KisShapeSelectionFactory();
    ~KisShapeSelectionFactory() override {}

    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override {
        Q_UNUSED(documentResources);
        return 0;
    }

    bool supports(const KoXmlElement & e, KoShapeLoadingContext &context) const override {
        Q_UNUSED(e);
        Q_UNUSED(context);
        return false;
    }
};

#endif
