/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    KisReferenceImagesDecoration(QPointer<KisView> parent, KisDocument *document, bool viewReady = true);
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

    void setReferenceImageLayer(KisSharedPtr<KisReferenceImagesLayer> layer, bool updateCanvas);
};

#endif // KISREFERENCEIMAGESDECORATION_H
