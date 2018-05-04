/*
 * Copyright (C) 2016 Miroslav Talasek <miroslav.talasek@seznam.cz>
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

#include "waveletdecompose.h"

#include <QMap>
#include <QPointer>
#include <QHash>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColor.h>

#include <kis_debug.h>
#include <kis_types.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_wavelet_kernel.h>
#include <kis_action.h>
#include <KisDocument.h>
#include <kis_node.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_random_accessor_ng.h>
#include "dlg_waveletdecompose.h"
#include "kis_node_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_undo_adapter.h"

#include <KoUpdater.h>
#include <KoProgressUpdater.h>



K_PLUGIN_FACTORY_WITH_JSON(WaveletDecomposeFactory, "kritawaveletdecompose.json", registerPlugin<WaveletDecompose>();)

WaveletDecompose::WaveletDecompose(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action  = createAction("waveletdecompose");
    connect(action, SIGNAL(triggered()), this, SLOT(slotWaveletDecompose()));
}

WaveletDecompose::~WaveletDecompose()
{
}

void WaveletDecompose::slotWaveletDecompose()
{
    DlgWaveletDecompose dlg(viewManager()->mainWindow(), "WaveletDecompose");
        
    if (dlg.exec() == QDialog::Accepted) {

        QApplication::setOverrideCursor(Qt::WaitCursor);

        QPointer<KoUpdater> updater = viewManager()->createUnthreadedUpdater(i18n("Wavelet Decompose"));

        KisImageSP image = viewManager()->image();
        if (!image) return;

        if (!viewManager()->blockUntilOperationsFinished(image)) return;

        image->barrierLock();

        KisPaintDeviceSP projection = new KisPaintDevice(*(image->projection()));
        if (!projection) return;
       
        const KoColorSpace *cs = projection->colorSpace();

        const KoCompositeOp* op = cs->compositeOp(COMPOSITE_GRAIN_EXTRACT);
        
        int scales = dlg.scales();
        
        QList<KisPaintDeviceSP> results;
        const QBitArray flags(0);
        
        QRect rc = image->bounds();
        
        KisPaintDeviceSP original = projection;
        
        //main loop
        for(int level = 0; level < scales; ++level){
        
            //copy original
            KisPaintDeviceSP blur = new KisPaintDevice(*original);
           
            //blur it
            KisWaveletKernel::applyWavelet(blur, rc, 1 << level, 1 << level, flags, 0);
       
            //do grain extract blur from original
            KisPainter painter(original);
            painter.setCompositeOp(op);
            painter.bitBlt(0, 0, blur, 0, 0, rc.width(), rc.height());
            painter.end();
        
            //original is new scale and blur is new original
            results << original;
            original = blur;
            updater->setProgress((level * 100) / scales);
        }
        //add new layers
        KisUndoAdapter *undo = image->undoAdapter();
        undo->beginMacro(kundo2_i18n("Wavelet decompose"));
        
        KisNodeCommandsAdapter adapter(viewManager());
        
        KisGroupLayerSP baseGroup = image->rootLayer();

        //add layer goup              
        KisGroupLayerSP grp = new KisGroupLayer(image, i18n("Wavelet decompose"), OPACITY_OPAQUE_U8);
        adapter.addNode(grp, baseGroup, baseGroup->lastChild());
        baseGroup = grp;

        //add scales
        int i = 1;
        const KoCompositeOp* op2 = cs->compositeOp(COMPOSITE_GRAIN_MERGE);
        Q_FOREACH (const KisPaintDeviceSP &l, results) {
            KisPaintLayerSP paintLayer = new KisPaintLayer(image, QStringLiteral("Scale %1").arg(i), OPACITY_OPAQUE_U8, l);
            adapter.addNode(paintLayer, baseGroup, 0);
            adapter.setCompositeOp(paintLayer, op2);
            ++i;
        }

        //add residual
        KisPaintLayerSP paintLayer = new KisPaintLayer(image, "Residual", OPACITY_OPAQUE_U8, original);
        adapter.addNode(paintLayer, baseGroup, 0);
        
        undo->endMacro();
        updater->setProgress(100);
        image->unlock();
        image->setModified();
    }
        
    QApplication::restoreOverrideCursor();
}

#include "waveletdecompose.moc"
