/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_SHAPE_LAYER_PASTE_H
#define KIS_SHAPE_LAYER_PASTE_H
#include <KoDrag.h>
#include <KoOdf.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfPaste.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoShapeOdfSaveHelper.h>
#include "kis_shape_layer.h"
#include "kis_shape_selection.h"

class KisShapeLayerShapePaste : public KoOdfPaste
{
public:
    KisShapeLayerShapePaste(KoShapeLayer* _container, KoShapeBasedDocumentBase* _controller)
            : m_container(_container)
            , m_controller(_controller) {
    }

    virtual ~KisShapeLayerShapePaste() {
    }

    virtual bool process(const KoXmlElement & body, KoOdfReadStore & odfStore) {
        KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
        KoShapeLoadingContext context(loadingContext, m_controller ? m_controller->resourceManager() : 0);
        KoXmlElement child;
        forEachElement(child, body) {
            KoShape * shape = KoShapeRegistry::instance()->createShapeFromOdf(child, context);
            if (shape) {
                KisShapeLayer* shapeLayer = dynamic_cast<KisShapeLayer*>(m_container);
                if (shapeLayer) {
                    //don't update as the setDirty call would create shared pointer that would delete the layer
                    shapeLayer->addShape(shape);
                } else {
                    KisShapeSelection* shapeSelection = dynamic_cast<KisShapeSelection*>(m_container);
                    shapeSelection->addShape(shape);
                }
            }
        }
        return true;
    }
private:
    KoShapeLayer* m_container;
    KoShapeBasedDocumentBase* m_controller;
};
#endif
