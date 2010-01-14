/*
 * backgrounds.cc -- Part of Krita
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

#include "backgrounds.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#include <kis_config.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_background.h>
#include "dlg_backgrounds.h"

K_PLUGIN_FACTORY(BackgroundsFactory, registerPlugin<Backgrounds>();)
K_EXPORT_PLUGIN(BackgroundsFactory("krita"))

Backgrounds::Backgrounds(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setComponentData(BackgroundsFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/backgrounds.rc"), true);

        KAction *action  = new KAction(i18n("Select Image Background..."), this);
        actionCollection()->addAction("backgrounds", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotBackgrounds()));

        m_view = (KisView2*) parent;
    }
}

Backgrounds::~Backgrounds()
{
    m_view = 0;
}

void Backgrounds::slotBackgrounds()
{

    DlgBackgrounds * dlgBackgrounds = new DlgBackgrounds(m_view);
    dlgBackgrounds->setObjectName("Backgrounds");
    Q_CHECK_PTR(dlgBackgrounds);

    if (dlgBackgrounds->exec() == QDialog::Accepted) {
        m_view->image()->setBackgroundPattern(dlgBackgrounds->background());
    }

    delete dlgBackgrounds;
}

#include "backgrounds.moc"
