/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_view.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>

#include "kis_dropshadow_plugin.h"
#include "kis_dropshadow.h"
#include "dlg_dropshadow.h"

K_EXPORT_COMPONENT_FACTORY( kritadropshadow, KGenericFactory<KisDropshadowPlugin>( "krita" ) )

KisDropshadowPlugin::KisDropshadowPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView") ) {

        setInstance(KGenericFactory<KisDropshadowPlugin>::instance());
        setXMLFile(locate("data","kritaplugins/dropshadow.rc"), true);

        m_view = (KisView*) parent;
        (void) new KAction(i18n("Add Drop Shadow..."), 0, 0, this, SLOT(slotDropshadow()), actionCollection(), "dropshadow");
    }
}

KisDropshadowPlugin::~KisDropshadowPlugin()
{
}

void KisDropshadowPlugin::slotDropshadow()
{
    KisImageSP image = m_view->canvasSubject()->currentImg();
    if (!image) return;

    KisPaintDeviceSP dev = image->activeDevice();
    if (!dev) return;

    DlgDropshadow * dlgDropshadow = new DlgDropshadow(dev->colorSpace()->id().name(),
                                                      image->colorSpace()->id().name(),
                                                      m_view, "Dropshadow");
    Q_CHECK_PTR(dlgDropshadow);

    dlgDropshadow->setCaption(i18n("Drop Shadow"));

    if (dlgDropshadow->exec() == QDialog::Accepted) {

        KisDropshadow dropshadow(m_view);
        dropshadow.dropshadow(m_view->canvasSubject()->progressDisplay(),
                           dlgDropshadow->getXOffset(),
                           dlgDropshadow->getYOffset(),
                           dlgDropshadow->getBlurRadius(),
                           dlgDropshadow->getShadowColor(),
                           dlgDropshadow->getShadowOpacity(),
                           dlgDropshadow->allowResizingChecked());

    }

    delete dlgDropshadow;

}

#include "kis_dropshadow_plugin.moc"
