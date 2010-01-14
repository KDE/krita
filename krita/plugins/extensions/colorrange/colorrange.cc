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
#include <kiconloader.h>
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

#include "dlg_colorrange.h"

K_PLUGIN_FACTORY(ColorRangeFactory, registerPlugin<ColorRange>();)
K_EXPORT_PLUGIN(ColorRangeFactory("krita"))

ColorRange::ColorRange(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setComponentData(ColorRangeFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/colorrange.rc"),
                   true);
        m_view = dynamic_cast<KisView2*>(parent);
        QAction *action  = new KAction(i18n("&Color Range..."), this);
        actionCollection()->addAction("colorrange", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotActivated()));
        m_view->selectionManager()->addSelectionAction(action);
    }
}

ColorRange::~ColorRange()
{
}

void ColorRange::slotActivated()
{
    KisPaintDeviceSP layer = m_view->activeDevice();
    if (!layer) return;

    DlgColorRange * dlgColorRange = new DlgColorRange(m_view, layer, m_view, "ColorRange");
    Q_CHECK_PTR(dlgColorRange);

    dlgColorRange->exec();
}

#include "colorrange.moc"

