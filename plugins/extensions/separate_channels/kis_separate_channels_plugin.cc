/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                           dlgSeparate->getDownscale(),
                           dlgSeparate->getToColor(),
                           dlgSeparate->getActivateCurrentChannel());
        QApplication::restoreOverrideCursor();
    }

    delete dlgSeparate;

}

#include "kis_separate_channels_plugin.moc"
