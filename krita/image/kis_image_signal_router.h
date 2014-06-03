/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_IMAGE_SIGNAL_ROUTER_H
#define __KIS_IMAGE_SIGNAL_ROUTER_H

#include <QObject>
#include <QVector>
#include "krita_export.h"
#include "kis_types.h"
#include "kis_group_layer.h"


class KoColorSpace;
class KoColorProfile;

enum KisImageSignalTypeEnum {
    LayersChangedSignal,
    ModifiedSignal,
    SizeChangedSignal,
    ProfileChangedSignal,
    ColorSpaceChangedSignal,
    ResolutionChangedSignal
};

/**
 * A special signal which handles stillPoint capabilities of the image
 *
 * \see KisImage::sigSizeChanged()
 */
struct ComplexSizeChangedSignal {
    ComplexSizeChangedSignal() {}
    ComplexSizeChangedSignal(QPointF _oldStillPoint, QPointF _newStillPoint)
        : oldStillPoint(_oldStillPoint),
          newStillPoint(_newStillPoint)
    {
    }

    /**
     * A helper method calculating the still points from image areas
     * we process. It works as if the source image was "cropped" by \p
     * portionOfOldImage, and this portion formed the new image of size
     * \p transformedIntoImageOfSize.
     *
     * Note, that \p portionOfTheImage may be equal to the image bounds().
     */
    ComplexSizeChangedSignal(const QRect &portionOfOldImage, const QSize &transformedIntoImageOfSize)
    {
        oldStillPoint = QRectF(portionOfOldImage).center();
        newStillPoint = QRectF(QPointF(), QSizeF(transformedIntoImageOfSize)).center();
    }

    ComplexSizeChangedSignal inverted() const {
        return ComplexSizeChangedSignal(newStillPoint, oldStillPoint);
    }

    QPointF oldStillPoint;
    QPointF newStillPoint;
};

struct KisImageSignalType {
    KisImageSignalType() {}
    KisImageSignalType(KisImageSignalTypeEnum _id)
    : id(_id)
    {
    }

    KisImageSignalType(ComplexSizeChangedSignal signal)
        : id(SizeChangedSignal),
          sizeChangedSignal(signal)
    {
    }

    KisImageSignalType inverted() const {
        KisImageSignalType t;
        t.id = id;
        t.sizeChangedSignal = sizeChangedSignal.inverted();
        return t;
    }

    KisImageSignalTypeEnum id;
    ComplexSizeChangedSignal sizeChangedSignal;
};

typedef QVector<KisImageSignalType> KisImageSignalVector;

class KRITAIMAGE_EXPORT KisImageSignalRouter : public QObject
{
    Q_OBJECT

public:
    KisImageSignalRouter(KisImageWSP image);
    ~KisImageSignalRouter();

    void emitNotification(KisImageSignalType type);
    void emitNotifications(KisImageSignalVector notifications);

    void emitNodeChanged(KisNodeSP node);
    void emitNodeHasBeenAdded(KisNode *parent, int index);
    void emitAboutToRemoveANode(KisNode *parent, int index);

private slots:
    void slotNotification(KisImageSignalType type);

signals:

    void sigNotification(KisImageSignalType type);

    // Notifications
    void sigImageModified();

    void sigSizeChanged(const QPointF &oldStillPoint, const QPointF &newStillPoint);
    void sigProfileChanged(const KoColorProfile *  profile);
    void sigColorSpaceChanged(const KoColorSpace*  cs);
    void sigResolutionChanged(double xRes, double yRes);

    // Graph change signals
    void sigNodeChanged(KisNodeSP node);
    void sigNodeAddedAsync(KisNodeSP node);
    void sigRemoveNodeAsync(KisNodeSP node);
    void sigLayersChangedAsync();

private:
    KisImageWSP m_image;
};

#endif /* __KIS_IMAGE_SIGNAL_ROUTER_H */
