/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SELECTIONDECORATOR_H
#define SELECTIONDECORATOR_H

#include <KoViewConverter.h>
#include <KoFlake.h>

#include "KoShapeMeshGradientHandles.h"
#include "KisReferenceImagesLayer.h"

#include <QPainter>
#include <QPointer>

class KoSelection;
class KoCanvasResourceProvider;

static const struct DecoratorIconPositions {
    QPoint uiOffset = QPoint(0, 40);
} decoratorIconPositions;

/**
 * The SelectionDecorator is used to paint extra user-interface items on top of a selection.
 */
class SelectionDecorator
{
public:
    /**
     * Constructor.
     * @param arrows the direction that needs highlighting. (currently unused)
     * @param rotationHandles if true; the rotation handles will be drawn
     * @param shearHandles if true; the shearhandles will be drawn
     */
    SelectionDecorator(KoCanvasResourceProvider *resourceManager);
    ~SelectionDecorator() {}

    /**
     * paint the decortations.
     * @param painter the painter to paint to.
     * @param converter to convert between internal and view coordinates.
     */
    void paint(QPainter &painter, const KoViewConverter &converter);

    /**
     * set the selection that is to be painted.
     * @param selection the current selection.
     */
    void setSelection(KoSelection *selection);

    /**
     * set the radius of the selection handles
     * @param radius the new handle radius
     */
    void setHandleRadius(int radius);

    /**
     * Set true if you want to render gradient handles on the canvas.
     * Default value: false
     */
    void setShowFillGradientHandles(bool value);

    /**
     * Set true if you want to render gradient handles on the canvas.
     * Default value: false
     */
    void setShowStrokeFillGradientHandles(bool value);

    void setShowFillMeshGradientHandles(bool value);
    void setCurrentMeshGradientHandles(const KoShapeMeshGradientHandles::Handle &selectedHandle,
                                       const KoShapeMeshGradientHandles::Handle &hoveredHandle);

    void setForceShapeOutlines(bool value);

    void setReferenceImagesLayer(KisSharedPtr<KisReferenceImagesLayer> layer);

private:
    void paintGradientHandles(KoShape *shape, KoFlake::FillVariant fillVariant, QPainter &painter, const KoViewConverter &converter);

    void paintMeshGradientHandles(KoShape *shape, KoFlake::FillVariant fillVariant, QPainter &painter, const KoViewConverter &converter);

private:
    KoFlake::AnchorPosition m_hotPosition;
    KoSelection *m_selection;
    KoShapeMeshGradientHandles::Handle m_currentHoveredMeshHandle;
    KoShapeMeshGradientHandles::Handle m_selectedMeshHandle;
    KisSharedPtr<KisReferenceImagesLayer> referenceImagesLayer;
    int m_handleRadius;
    int m_lineWidth;
    bool m_showFillGradientHandles;
    bool m_showStrokeFillGradientHandles;
    bool m_showFillMeshGradientHandles;
    bool m_forceShapeOutlines;
};

#endif
