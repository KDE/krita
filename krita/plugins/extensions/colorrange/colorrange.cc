/*
 * colorrange.h -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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

#include "colorrange.h"


#include <math.h>

#include <stdlib.h>

#include <QSlider>
#include <QPoint>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kactioncollection.h>

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_selection_tool_helper.h"
#include "kis_canvas2.h"
#include "kis_iterator_ng.h"

#include "dlg_colorrange.h"
#include <KoColorSpace.h>

K_PLUGIN_FACTORY(ColorRangeFactory, registerPlugin<ColorRange>();)
K_EXPORT_PLUGIN(ColorRangeFactory("krita"))

ColorRange::ColorRange(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setXMLFile(KStandardDirs::locate("data", "kritaplugins/colorrange.rc"),
                   true);
        m_view = dynamic_cast<KisView2*>(parent);
        m_selectRange = new KAction(i18n("Select from Color Range..."), this);
        actionCollection()->addAction("colorrange", m_selectRange);
        connect(m_selectRange, SIGNAL(triggered()), this, SLOT(slotActivated()));
        m_view->selectionManager()->addSelectionAction(m_selectRange);

        m_selectOpaque  = new KAction(i18n("Select Opaque"), this);
        actionCollection()->addAction("selectopaque", m_selectOpaque);
        connect(m_selectOpaque, SIGNAL(triggered()), this, SLOT(selectOpaque()));
        m_view->selectionManager()->addSelectionAction(m_selectOpaque);

        connect(m_view->selectionManager(), SIGNAL(signalUpdateGUI()),
                SLOT(slotUpdateGUI()));
    }
}

ColorRange::~ColorRange()
{
}

void ColorRange::slotUpdateGUI()
{
    bool enable = m_view->selectionEditable();
    m_selectRange->setEnabled(enable);
    m_selectOpaque->setEnabled(enable);
}

void ColorRange::slotActivated()
{
    DlgColorRange *dlgColorRange = new DlgColorRange(m_view, m_view);
    Q_CHECK_PTR(dlgColorRange);

    dlgColorRange->exec();
}

void ColorRange::selectOpaque()
{
    KisCanvas2 * canvas = m_view->canvasBase();
    if (!canvas)
        return;
    
    KisNodeSP node = m_view->activeNode();
    if(!node)
        return;
    
    KisPaintDeviceSP device = node->paintDevice();
    if (!device) return;
    
    KisSelectionToolHelper helper(canvas, node, i18n("Select Opaque"));
    
    qint32 x, y, w, h;
    QRect rc = device->exactBounds();
    x = rc.x();
    y = rc.y();
    w = rc.width();
    h = rc.height();
    
    const KoColorSpace * cs = device->colorSpace();
    KisPixelSelectionSP tmpSel = KisPixelSelectionSP(new KisPixelSelection());

    KisHLineConstIteratorSP deviter = device->createHLineConstIteratorNG(x, y, w);
    KisHLineIteratorSP selIter = tmpSel ->createHLineIteratorNG(x, y, w);

    for (int row = y; row < h + y; ++row) {
        do {
            *selIter->rawData() = cs->opacityU8(deviter->oldRawData());
        } while (deviter->nextPixel() && selIter->nextPixel());
        deviter->nextRow();
        selIter->nextRow();
    }

    helper.selectPixelSelection(tmpSel, SELECTION_ADD);
}

#include "colorrange.moc"

