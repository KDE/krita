/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISREFERENCEIMAGE_H
#define KISREFERENCEIMAGE_H

#include <QSharedDataPointer>

#include <kundo2command.h>
#include <kritaui_export.h>
#include <KoTosContainer.h>
#include <KoColor.h>
#include <KoCanvasBase.h>

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

    struct KRITAUI_EXPORT CropReferenceImage : public KUndo2Command {
        KisReferenceImage *referenceImage;
        QImage oldImage;
        QRectF imageRect;
        QRectF newRect;
        QSizeF oldShapeSize;
        QPointF oldPos;

        explicit CropReferenceImage(KoShape *image, QRectF rect, KUndo2Command *parent = 0);
        int id() const override;
        void undo() override;
        void redo() override;
        bool mergeWith(const KUndo2Command *command) override;
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

    void shapeChanged(ChangeType type, KoShape *shape = 0) override;

    void setSaturation(qreal saturation);
    qreal saturation() const;

    void setEmbed(bool embed);
    bool embed();
    bool hasLocalFile();

    void setFilename(const QString &filename);
    QString filename() const;
    QString internalFile() const;

    void paint(QPainter &gc, KoShapePaintingContext &paintcontext) const override;

    QColor getPixel(QPointF position);

    void saveXml(QDomDocument &document, QDomElement &parentElement, int id);
    bool saveImage(KoStore *store) const;

    static KisReferenceImage * fromXml(const QDomElement &elem);
    bool loadImage(KoStore *store);

    QImage image();
    void setImage(QImage);
    void reloadImage();

    bool crop();
    QRectF cropRect();
    void scaleCropRect(qreal scaleX, qreal scaleY);

    void setCrop(bool, QRectF);
    void setCropRect(QRectF);
    qreal addCanvasTransformation(KisCanvas2 *kisCanvas);

    bool pinRotate();
    void setPinRotate(bool);
    bool pinMirror();
    void setPinMirror(bool);
    bool pinPosition();
    void setPinPosition(bool);
    bool pinZoom();
    void setPinZoom(bool);
    bool pinAll();
    void setPinAll(bool);

private:
    struct Private;
    QSharedDataPointer<Private> d;
};

#endif // KISREFERENCEIMAGE_H
