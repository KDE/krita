/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_IMAGE_PATCH_H_
#define KIS_IMAGE_PATCH_H_

#include <QPainter>
#include <QImage>
#include <kis_types.h>

#define BORDER_SIZE(scale) (ceil(0.5/scale))


class KisImagePatch
{
public:
    /**
     * A default constructor initializing invalid patch
     */
    KisImagePatch();

    /**
     * Initializes a new patch with given values.
     * Be careful, because the constructor does not fill
     * QImage of the patch, as the patch rect is not known yet
     *
     * \see setImage
     */
    KisImagePatch(QRect imageRect, qint32 borderWidth,
                  qreal scaleX, qreal scaleY);

    /**
     * Sets the image of the patch
     * Should be called right after the constructor
     * to finish initializing the object
     */
    void setImage(QImage image);

    /**
     * prescale the patch image. Call after setImage().
     * This ensures that we use the QImage smoothscale method, not the QPainter scaling,
     * which is far inferior.
     */
    void preScale(const QRectF &dstRect);

    /**
     * Returns the rect of KisImage covered by the image
     * of the patch (in KisImage pixels)
     *
     * \see m_patchRect
     */
    QRect patchRect();

    /**
     * Draws an m_interestRect of the patch onto @p gc
     * By the way it fits this rect into @p dstRect
     * @p renderHints are directly transmitted to QPainter
     */
    void drawMe(QPainter &gc,
                const QRectF &dstRect,
                QPainter::RenderHints renderHints);

    /**
     * Checks whether the patch can be used for drawing the image
     */
    bool isValid();

private:
    /**
     * The scale of the image stored in the patch
     */
    qreal m_scaleX;
    qreal m_scaleY;

    /**
     * The rect of KisImage covered by the image
     * of the patch (in KisImage pixels)
     */
    QRect m_patchRect;

    /**
     * The rect that was requested during creation
     * of the patch. It equals to patchRect withount
     * borders
     * These borders are introduced for more accurate
     * smooth scaling to reduce border effects
     * (IN m_image PIXELS, relative to m_image's topLeft);

     */
    QRectF m_interestRect;

    QImage m_image;
    bool m_isScaled;
};

#endif /* KIS_IMAGE_PATCH_H_ */
