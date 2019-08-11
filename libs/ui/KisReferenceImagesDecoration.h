/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISREFERENCEIMAGESDECORATION_H
#define KISREFERENCEIMAGESDECORATION_H

#include <QObject>
#include <QPainter>
#include <QRectF>
#include <kis_shared_ptr.h>

class KisReferenceImagesDecoration;
class KisReferenceImagesLayer;
typedef KisSharedPtr<KisReferenceImagesDecoration> KisReferenceImagesDecorationSP;

#include <kis_coordinates_converter.h>
#include <kis_canvas_decoration.h>
#include <kis_canvas2.h>
#include <kis_types.h>

/**
 * @brief The KisReferenceImagesDecoration class draws the reference images on the canvas.
 * The document stores the list of reference images.
 */
class KisReferenceImagesDecoration : public KisCanvasDecoration
{
    Q_OBJECT
public:
    KisReferenceImagesDecoration(QPointer<KisView> parent, KisDocument *document);
    ~KisReferenceImagesDecoration() override;

    void addReferenceImage(KisReferenceImage *referenceImage);

    bool documentHasReferenceImages() const;

private Q_SLOTS:
    void slotNodeAdded(KisNodeSP);
    void slotReferenceImagesChanged(const QRectF &dirtyRect);

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateRect, const KisCoordinatesConverter *converter, KisCanvas2* canvas) override;

private:
    struct Private;
    const QScopedPointer<Private> d;

    void setReferenceImageLayer(KisSharedPtr<KisReferenceImagesLayer> layer);
};

#endif // KISREFERENCEIMAGESDECORATION_H
