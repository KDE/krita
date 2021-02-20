/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoJsonTrader.h>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QMessageBox>
#include <QUuid>
#include <kis_action.h>
#include <kis_config.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include "QMic.h"
#include "kis_qmic_interface.h"
#include "KisViewManager.h"

K_PLUGIN_FACTORY_WITH_JSON(QMicFactory, "kritaqmic.json", registerPlugin<QMic>();)

QMic::QMic(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    m_qmicAction = createAction("QMic");
    m_qmicAction->setActivationFlags(KisAction::ACTIVE_DEVICE);

    connect(m_qmicAction, SIGNAL(triggered()), this, SLOT(slotQMic()));

    m_againAction = createAction("QMicAgain");
    m_againAction->setActivationFlags(KisAction::ACTIVE_DEVICE);
    m_againAction->setEnabled(false);
    connect(m_againAction, SIGNAL(triggered()), this, SLOT(slotQMicAgain()));
}

void QMic::slotQMicAgain()
{
    slotQMic(true);
}

void QMic::slotQMic(bool again)
{
    m_qmicAction->setEnabled(false);
    m_againAction->setEnabled(false);

    // find the krita-gmic-qt plugin
    const auto offers = KoJsonTrader::instance()->query("Krita/GMic", QString());
    if (offers.isEmpty()) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("The GMic plugin is not installed or could not be loaded."));
        return;
    }

    for (const auto &loader : offers) {
        auto *factory = qobject_cast<KPluginFactory *>(loader->instance());
        if (!factory) {
            warnPlugins << "(GMic) This is not a Krita plugin: " << loader->fileName() << loader->errorString();

            continue;
        }

        auto *pluginBase = factory->create<QObject>(this);

        plugin = qobject_cast<KisQmicPluginInterface *>(pluginBase);

        if (!plugin) {
            warnPlugins << "(GMic) This is not a valid GMic-Qt plugin: " << loader->fileName();

            continue;
        }

        break;
    }

    if (!plugin) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Krita cannot launch the gmic-qt plugin. No bundled library found."));
        return;
    }

    qDeleteAll(offers);

    m_key = QUuid::createUuid().toString();
    auto image = std::make_shared<KisImageInterface>(this->viewManager().data());
    int status = plugin->launch(image, again);

    dbgPlugins << "pluginFinished" << status;
    delete plugin;
    plugin = nullptr;

    m_qmicAction->setEnabled(true);
    m_againAction->setEnabled(true);
}

#include "QMic.moc"
