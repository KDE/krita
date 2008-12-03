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
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include <krita_export.h>
#include "kis_global.h"
#include "kis_types.h"
#include "kis_node_visitor.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"
#include "kis_external_layer_iface.h"
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "filter/kis_filter.h"
#include "generator/kis_generator_layer.h"

// XXX: Make undoable (used to be in KisPaintDevice, should be in
// KisLayer)
#if 0
namespace
{

class KisPaintDeviceCommand : public QUndoCommand
{
public:
    KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice);
    virtual ~KisPaintDeviceCommand() {}

protected:
    void setUndo(bool undo);

    KisPaintDeviceSP m_paintDevice;
};

KisPaintDeviceCommand::KisPaintDeviceCommand(const QString& name, KisPaintDeviceSP paintDevice) :
        QUndoCommand(name), m_paintDevice(paintDevice)
{
}

void KisPaintDeviceCommand::setUndo(bool undo)
{
    if (m_paintDevice->undoAdapter()) {
        m_paintDevice->undoAdapter()->setUndo(undo);
    }
}

class KisConvertLayerTypeCmd : public KisPaintDeviceCommand
{

public:
    KisConvertLayerTypeCmd(KisPaintDeviceSP paintDevice,
                           KisDataManagerSP beforeData, const KoColorSpace * beforeColorSpace,
                           KisDataManagerSP afterData, const KoColorSpace * afterColorSpace)
            : KisPaintDeviceCommand(i18n("Convert Layer Type"), paintDevice) {
        m_beforeData = beforeData;
        m_beforeColorSpace = beforeColorSpace;
        m_afterData = afterData;
        m_afterColorSpace = afterColorSpace;
    }

    virtual ~KisConvertLayerTypeCmd() {
    }

public:
    virtual void redo() {
        setUndo(false);
        m_paintDevice->setDataManager(m_afterData, m_afterColorSpace);
        setUndo(true);
    }

    virtual void undo() {
        setUndo(false);
        m_paintDevice->setDataManager(m_beforeData, m_beforeColorSpace);
        setUndo(true);
    }

private:
    KisDataManagerSP m_beforeData;
    const KoColorSpace * m_beforeColorSpace;

    KisDataManagerSP m_afterData;
    const KoColorSpace * m_afterColorSpace;
};

}
#endif


class KRITAIMAGE_EXPORT KisColorSpaceConvertVisitor : public KisNodeVisitor
{
public:
    KisColorSpaceConvertVisitor(const KoColorSpace *dstColorSpace, KoColorConversionTransformation::Intent renderingIntent);
    virtual ~KisColorSpaceConvertVisitor();

public:

    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer* layer);
    bool visit(KisGeneratorLayer * layer);
    bool visit(KisExternalLayer *);

    bool visit(KisNode*) { return true; }
    bool visit(KisCloneLayer*) { return true; }
    bool visit(KisFilterMask*) { return true; }
    bool visit(KisTransparencyMask*) { return true; }
    bool visit(KisTransformationMask*) { return true; }
    bool visit(KisSelectionMask*) { return true; }


private:
    const KoColorSpace *m_dstColorSpace;
    KoColorConversionTransformation::Intent m_renderingIntent;
    QBitArray m_emptyChannelFlags;
};


#endif // KIS_COLORSPACE_CONVERT_VISITOR_H_

