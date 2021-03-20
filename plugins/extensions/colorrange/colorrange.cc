/*
 * colorrange.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <KisSignalMapper.h>

K_PLUGIN_FACTORY_WITH_JSON(ColorRangeFactory, "kritacolorrange.json", registerPlugin<ColorRange>();)


ColorRange::ColorRange(QObject *parent, const QVariantList &)
        : KisActionPlugin(parent)
{
    KisAction* action = createAction("colorrange");
    connect(action, SIGNAL(triggered()), this, SLOT(slotActivated()));


    KisSignalMapper *mapper = new KisSignalMapper(this);
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
    KisNodeSP node = viewManager()->activeNode();
    if (!node) return;

    viewManager()->selectionManager()->
        selectOpaqueOnNode(node, SelectionAction(id));
}

#include "colorrange.moc"

