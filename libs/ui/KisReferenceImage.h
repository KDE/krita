/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISREFERENCEIMAGE_H
#define KISREFERENCEIMAGE_H

#include <QSharedDataPointer>

#include <kundo2command.h>
#include <kritaui_export.h>
#include <KoTosContainer.h>
#include <KoColor.h>

class QImage;
class QPointF;
class QPainter;
class QRectF;
class KoStore;
class KisCoordinatesConverter;
class KisCanvas2;

/**
 * @brief The KisReferenceImage class represents a single reference image
 */
class KRITAUI_EXPORT KisReferenceImage : public KoTosContainer
{
public:
    struct KRITAUI_EXPORT SetSaturationCommand : public KUndo2Command {
        QVector<KisReferenceImage*> images;
        QVector<qreal> oldSaturations;
        qreal newSaturation;

        explicit SetSaturationCommand(const QList<KoShape *> &images, qreal newSaturation, KUndo2Command *parent = 0);
        void undo() override;
        void redo() override;
    };

    KisReferenceImage();
    KisReferenceImage(const KisReferenceImage &rhs);
    ~KisReferenceImage();

    KoShape *cloneShape() const override;

    /**
     * Load a reference image from specified file.
     * If parent is provided and the image cannot be loaded, a warning message will be displayed to user.
     * @return reference image or null if one could not be loaded
     */
    static KisReferenceImage * fromFile(const QString &filename, const KisCoordinatesConverter &converter, QWidget *parent /*= nullptr*/);
    static KisReferenceImage * fromClipboard(const KisCoordinatesConverter &converter);

    void setSaturation(qreal saturation);
    qreal saturation() const;

    void setEmbed(bool embed);
    bool embed();
    bool hasLocalFile();

    void setFilename(const QString &filename);
    QString filename() const;
    QString internalFile() const;

    void paint(QPainter &gc, KoShapePaintingContext &paintcontext) const override;

    bool loadOdf(const KoXmlElement &/*element*/, KoShapeLoadingContext &/*context*/) override { return false; }
    void saveOdf(KoShapeSavingContext &/*context*/) const override {}

    QColor getPixel(QPointF position);

    void saveXml(QDomDocument &document, QDomElement &parentElement, int id);
    bool saveImage(KoStore *store) const;

    static KisReferenceImage * fromXml(const QDomElement &elem);
    bool loadImage(KoStore *store);

private:
    struct Private;
    QSharedDataPointer<Private> d;
};

#endif // KISREFERENCEIMAGE_H
