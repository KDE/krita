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

#include <KoShapeContainer.h>
#include <KoShapeFactory.h>

#include <kis_selection_component.h>
#include <kis_types.h>

#include <krita_export.h>

class KisShapeSelectionCanvas;

/**
 *
 */
class KRITAUI_EXPORT KisShapeSelection : public QObject, public KoShapeContainer, public KisSelectionComponent
{
    Q_OBJECT

public:

    KisShapeSelection(KisImageSP image, KisSelectionSP selection);
    virtual ~KisShapeSelection();

    ///Not implemented
    virtual bool loadOdf(const KoXmlElement&, KoShapeLoadingContext&);

    ///Not implemented
    virtual void saveOdf(KoShapeSavingContext&) const;

    /**
     * Renders the shapes to a selection. This method should only be called
     * by KisSelection to update it's projection.
     *
     * @param projection the target selection 
     */
    virtual void renderToProjection(KisSelection* projection);
    virtual void renderToProjection(KisSelection* projection, const QRect& r);

    virtual void setDirty();

    virtual QPainterPath selectionOutline();

    KoShapeManager *shapeManager() const;

protected:
    virtual void paintComponent(QPainter& painter, const KoViewConverter& converter);

private:
    
    Q_DISABLE_COPY(KisShapeSelection)

    void renderSelection(KisSelection* projection, const QRect& r);

    KisImageSP m_image;
    QPainterPath m_outline;
    bool m_dirty;
    KisShapeSelectionCanvas* m_canvas;

    friend class KisShapeSelectionModel;
};


class KRITAUI_EXPORT KisShapeSelectionFactory : public KoShapeFactory
{
    Q_OBJECT
public:

    using KoShapeFactory::createDefaultShape;
    using KoShapeFactory::createShape;

    KisShapeSelectionFactory( QObject* parent);
    ~KisShapeSelectionFactory() {}

    KoShape* createDefaultShape() const;
    KoShape* createShape( const KoProperties* params ) const;
};

#endif
