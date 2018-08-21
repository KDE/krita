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

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_global.h"
#include "kis_types.h"
#include "KisViewManager.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_selection_tool_helper.h"
#include "kis_canvas2.h"
#include "kis_iterator_ng.h"
#include "kis_action.h"

#include "dlg_colorrange.h"
#include <KoColorSpace.h>
#include <QSignalMapper>

K_PLUGIN_FACTORY_WITH_JSON(ColorRangeFactory, "kritacolorrange.json", registerPlugin<ColorRange>();)


ColorRange::ColorRange(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction* action = createAction("colorrange");
    connect(action, SIGNAL(triggered()), this, SLOT(slotActivated()));


    QSignalMapper *mapper = new QSignalMapper(this);
    connect(mapper, SIGNAL(mapped(int)), SLOT(selectOpaque(int)));

    action  = createAction("selectopaque");
    mapper->setMapping(action, int(SELECTION_REPLACE));
    connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));

    action  = createAction("selectopaque_add");
    mapper->setMapping(action, int(SELECTION_ADD));
    connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));

    action  = createAction("selectopaque_subtract");
    mapper->setMapping(action, int(SELECTION_SUBTRACT));
    connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));

    action  = createAction("selectopaque_intersect");
    mapper->setMapping(action, int(SELECTION_INTERSECT));
    connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
}

ColorRange::~ColorRange()
{
}

void ColorRange::slotActivated()
{
    DlgColorRange *dlgColorRange = new DlgColorRange(viewManager(), viewManager()->mainWindow());
    Q_CHECK_PTR(dlgColorRange);

    dlgColorRange->exec();
}

void ColorRange::selectOpaque(int id)
{
    selectOpaqueImpl(SelectionAction(id));
}

void ColorRange::selectOpaqueImpl(SelectionAction action)
{
    KisCanvas2 *canvas = viewManager()->canvasBase();
    KisPaintDeviceSP device = viewManager()->activeNode()->projection();
    if (!device) device = viewManager()->activeNode()->paintDevice();
    if (!device) device = viewManager()->activeNode()->original();
    KIS_ASSERT_RECOVER_RETURN(canvas && device);

    QRect rc = device->exactBounds();
    if (rc.isEmpty()) return;

    /**
     * If there is nothing selected, just create a new selection
     */
    if (!canvas->imageView()->selection()) {
        action = SELECTION_REPLACE;
    }

    KUndo2MagicString actionName;

    switch (action) {
    case SELECTION_ADD:
        actionName = kundo2_i18n("Select Opaque (Add)");
        break;
    case SELECTION_SUBTRACT:
        actionName = kundo2_i18n("Select Opaque (Subtract)");
        break;
    case SELECTION_INTERSECT:
        actionName = kundo2_i18n("Select Opaque (Intersect)");
        break;
    default:
        actionName = kundo2_i18n("Select Opaque");
        break;
    }

    KisSelectionToolHelper helper(canvas, actionName);

    qint32 x, y, w, h;
    rc.getRect(&x, &y, &w, &h);

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

    tmpSel->invalidateOutlineCache();
    helper.selectPixelSelection(tmpSel, action);
}

#include "colorrange.moc"

