/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_COLORSPACE_CONVERT_VISITOR_H_
#define KIS_COLORSPACE_CONVERT_VISITOR_H_

#include <QBitArray>

#include <KoColorConversionTransformation.h>

#include "kis_global.h"
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"
#include "kis_external_layer_iface.h"
#include "kis_filter_configuration.h"
#include "kis_filter_registry.h"
#include "kis_filter.h"


// XXX: Make undoable (used to be in KisPaintDevice, should be in
// KisLayer)
#if 0
namespace {

    class KisPaintDeviceCommand : public QUndoCommand {
        typedef QUndoCommand super;

    public:
        KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice);
        virtual ~KisPaintDeviceCommand() {}

    protected:
        void setUndo(bool undo);

        KisPaintDeviceSP m_paintDevice;
    };

    KisPaintDeviceCommand::KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice) :
        super(name), m_paintDevice(paintDevice)
    {
    }

    void KisPaintDeviceCommand::setUndo(bool undo)
    {
        if (m_paintDevice->undoAdapter()) {
            m_paintDevice->undoAdapter()->setUndo(undo);
        }
    }

    class KisConvertLayerTypeCmd : public KisPaintDeviceCommand {
        typedef KisPaintDeviceCommand super;

    public:
        KisConvertLayerTypeCmd(KisPaintDeviceSP paintDevice,
                       KisDataManagerSP beforeData, KoColorSpace * beforeColorSpace,
                       KisDataManagerSP afterData, KoColorSpace * afterColorSpace
                ) : super(i18n("Convert Layer Type"), paintDevice)
            {
                m_beforeData = beforeData;
                m_beforeColorSpace = beforeColorSpace;
                m_afterData = afterData;
                m_afterColorSpace = afterColorSpace;
            }

        virtual ~KisConvertLayerTypeCmd()
            {
            }

    public:
        virtual void redo()
            {
                setUndo(false);
                m_paintDevice->setDataManager(m_afterData, m_afterColorSpace);
                setUndo(true);
            }

        virtual void undo()
            {
                setUndo(false);
                m_paintDevice->setDataManager(m_beforeData, m_beforeColorSpace);
                setUndo(true);
            }

    private:
        KisDataManagerSP m_beforeData;
        KoColorSpace * m_beforeColorSpace;

        KisDataManagerSP m_afterData;
        KoColorSpace * m_afterColorSpace;
    };

}
#endif


class KoColorSpaceConvertVisitor :public KisNodeVisitor {
public:
    KoColorSpaceConvertVisitor(KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent);
    virtual ~KoColorSpaceConvertVisitor();

public:

    bool visit( KisExternalLayer * )
        {
            return true;
        }

    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer* layer);

private:
    KoColorSpace *m_dstColorSpace;
    KoColorConversionTransformation::Intent m_renderingIntent;
    QBitArray m_emptyChannelFlags;
};

KoColorSpaceConvertVisitor::KoColorSpaceConvertVisitor(KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) :
    KisNodeVisitor(),
    m_dstColorSpace(dstColorSpace),
    m_renderingIntent(renderingIntent)
{
}

KoColorSpaceConvertVisitor::~KoColorSpaceConvertVisitor()
{
}

bool KoColorSpaceConvertVisitor::visit(KisGroupLayer * layer)
{
    // Clear the projection, we will have to re-render everything.
    // The image is already set to the new colorspace, so this'll work.
    layer->resetProjection();
    layer->setChannelFlags( m_emptyChannelFlags );
    KisLayerSP child = dynamic_cast<KisLayer*>( layer->firstChild().data() );
    while (child) {
        child->accept(*this);
        child = dynamic_cast<KisLayer*>( child->nextSibling().data() );
    }
    layer->setDirty();
    layer->setCompositeOp( m_dstColorSpace->compositeOp( layer->compositeOp()->id() ) );
    return true;
}

bool KoColorSpaceConvertVisitor::visit(KisPaintLayer *layer)
{
    layer->paintDevice()->convertTo(m_dstColorSpace, m_renderingIntent);
    layer->setChannelFlags( m_emptyChannelFlags );
    layer->setDirty();
    layer->setCompositeOp( m_dstColorSpace->compositeOp( layer->compositeOp()->id() ) );
    return true;
}


bool KoColorSpaceConvertVisitor::visit(KisAdjustmentLayer * layer)
{
    if (layer->filter()->name() == "perchannel") {
        // Per-channel filters need to be reset because of different number
        // of channels. This makes undo very tricky, but so be it.
        // XXX: Make this more generic for after 1.6, when we'll have many
        // channel-specific filters.
        KisFilterSP f = KisFilterRegistry::instance()->value("perchannel");
        layer->setFilter(f->defaultConfiguration(0));
    }
    layer->setChannelFlags( m_emptyChannelFlags );
    layer->resetCache();
    layer->setDirty();
    return true;
}

#endif // KIS_COLORSPACE_CONVERT_VISITOR_H_

