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

#include "kis_separate_channels_plugin.h"

#include <QApplication>

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include <KoColorSpace.h>

#include <KisViewManager.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_statusbar.h>
#include <kis_node_manager.h>
#include <widgets/kis_progress_widget.h>
#include <kis_action.h>

#include "kis_channel_separator.h"
#include "dlg_separate.h"

K_PLUGIN_FACTORY_WITH_JSON(KisSeparateChannelsPluginFactory, "kritaseparatechannels.json", registerPlugin<KisSeparateChannelsPlugin>();)

KisSeparateChannelsPlugin::KisSeparateChannelsPlugin(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisAction *action  = createAction("separate");
    connect(action, SIGNAL(triggered(bool)), SLOT(slotSeparate()));
}

KisSeparateChannelsPlugin::~KisSeparateChannelsPlugin()
{
}

void KisSeparateChannelsPlugin::slotSeparate()
{
    KisImageSP image = viewManager()->image();
    if (!image) return;

    KisLayerSP l = viewManager()->nodeManager()->activeLayer();
    if (!l) return;

    KisPaintDeviceSP dev = l->paintDevice();
    if (!dev) return;

    DlgSeparate * dlgSeparate = new DlgSeparate(dev->colorSpace()->name(),
                                                image->colorSpace()->name(), viewManager()->mainWindow(), "Separate");
    Q_CHECK_PTR(dlgSeparate);

    dlgSeparate->setCaption(i18n("Separate Image"));

    // If we're 8-bits, disable the downscale option
    if (dev->pixelSize() == dev->channelCount()) {
        dlgSeparate->enableDownscale(false);
    }

    if (dlgSeparate->exec() == QDialog::Accepted) {

        QApplication::setOverrideCursor(Qt::BusyCursor);

        KisChannelSeparator separator(viewManager());
        separator.separate(viewManager()->createUnthreadedUpdater(i18n("Separate Image")),
                           dlgSeparate->getAlphaOptions(),
                           dlgSeparate->getSource(),
                           dlgSeparate->getOutput(),
                           dlgSeparate->getDownscale(),
                           dlgSeparate->getToColor());
        QApplication::restoreOverrideCursor();
    }

    delete dlgSeparate;

}

#include "kis_separate_channels_plugin.moc"
