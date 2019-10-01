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

