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

#include "kis_selection_component.h"

#include <krita_export.h>

class KRITAUI_EXPORT KisShapeSelection : public QObject, public KoShapeContainer, public KisSelectionComponent
{
    Q_OBJECT

public:
    KisShapeSelection(KisImageSP image, KisPaintDeviceSP dev);
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

    virtual void setDirty();

protected:
    virtual QPainterPath selectionOutline();
    virtual void paintComponent(QPainter& painter, const KoViewConverter& converter);

private slots:
    void repaintTimerEvent();

private:
    int m_dashOffset;
    QTimer* m_timer;
    KisImageSP m_image;
    KisPaintDeviceSP m_parentPaintDevice;
    QPainterPath m_outline;
    bool m_dirty;

    friend class KisShapeSelectionModel;
};

class KRITAUI_EXPORT KisShapeSelectionFactory : public KoShapeFactory
{
    Q_OBJECT
public:
    KisShapeSelectionFactory( QObject* parent);
    ~KisShapeSelectionFactory() {}

    KoShape* createDefaultShape() const;
    KoShape* createShape( const KoProperties* params ) const;
};

#endif
