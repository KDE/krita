/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KIS_SHAPE_LAYER_H_
#define KIS_SHAPE_LAYER_H_

#include <KoShapeLayer.h>

#include <kis_types.h>
#include <kis_external_layer_iface.h>
#include <krita_export.h>

class QRect;
class QPainter;
class QIcon;
class QRect;
class QDomDocument;
class QDomElement;
class QString;
class KoStore;
class KoViewConverter;
class KoShapeControllerBase;

const QString KIS_SHAPE_LAYER_ID = "KisShapeLayer";
/**
   A KisShapeLayer contains any number of non-krita flakes, such as
   path shapes, text shapes and anything else people come up with.

   The KisShapeLayer has a shapemanager and a canvas of its own. The
   canvas paints onto the projection, and the projection is what we
   render in Krita. This means that no matter how many views you have,
   you cannot have a different view on your shapes per view.

   XXX: what about removing shapes?
*/
class KRITAUI_TEST_EXPORT KisShapeLayer : public KisExternalLayer, public KoShapeLayer
{
    Q_OBJECT

public:

    KisShapeLayer(KoShapeContainer * parent, KoShapeControllerBase* shapeController, KisImageWSP image, const QString &name, quint8 opacity);
    KisShapeLayer(const KisShapeLayer& _rhs);
    virtual ~KisShapeLayer();
private:
    void initShapeLayer(KoShapeControllerBase* controller);
public:
    KisNodeSP clone() const {
        return KisNodeSP(new KisShapeLayer(*this));
    }
    bool allowAsChild(KisNodeSP) const;

public:

    // KoShape overrides
    bool isSelectable() const {
        return false;
    }

    // KoShapeContainer overrides
    void addChild(KoShape *object);
    void removeChild(KoShape *object);

    // KisExternalLayer implementation
    QIcon icon() const;

    KisPaintDeviceSP original() const;
    KisPaintDeviceSP paintDevice() const;

    QRect repaintOriginal(KisPaintDeviceSP original,
                          const QRect& rect);

    qint32 x() const;
    qint32 y() const;
    void setX(qint32);
    void setY(qint32);

    bool accept(KisNodeVisitor&);

    KoShapeManager *shapeManager() const;

    bool saveLayer(KoStore * store) const;
    bool loadLayer(KoStore* store);
    
    QUndoCommand* crop(const QRect & rect);
    
    QUndoCommand* transform(double  xscale, double  yscale, double  xshear, double  yshear, double angle, qint32  translatex, qint32  translatey);

public slots:
    void selectionChanged();

signals:
    void selectionChanged(QList<KoShape*> shape);

private:
    class Private;
    Private * const m_d;
};

#endif
