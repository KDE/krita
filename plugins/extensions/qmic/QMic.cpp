/*
 * SPDX-FileCopyrightText: 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "QMic.h"

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QMultiMap>
#include <QProcess>
#include <QSharedMemory>
#include <QSharedPointer>
#include <QUuid>
#include <QVBoxLayout>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoDialog.h>

#include <KisPart.h>
#include <KisViewManager.h>
#include <kis_action.h>
#include <kis_algebra_2d.h>
#include <kis_config.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_preference_set_registry.h>
#include <kis_selection.h>

#include "KritaGmicPluginInterface.h"
#include "kis_image_interface.h"
#include "kis_import_qmic_processing_visitor.h"
#include "kis_input_output_mapper.h"
#include "kis_qmic_simple_convertor.h"
#include <PluginSettings.h>
#include <kis_image_barrier_locker.h>
#include <qsize.h>

#include "kis_qmic_applicator.h"

K_PLUGIN_FACTORY_WITH_JSON(QMicFactory, "kritaqmic.json", registerPlugin<QMic>();)

QMic::QMic(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    PluginSettingsFactory *settingsFactory = new PluginSettingsFactory();
    preferenceSetRegistry->add("QMicPluginSettingsFactory", settingsFactory);

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
    QString pluginPath = PluginSettings::gmicQtPath();
    if (pluginPath.isEmpty() || !QFileInfo(pluginPath).exists() || !QFileInfo(pluginPath).isFile()) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Krita cannot find the gmic-qt plugin. You can set the location of the gmic-qt plugin in Settings/Configure Krita."));
        return;
    }

    QPluginLoader loader(pluginPath);

    if (!loader.load()) {
        QMessageBox msgBox(qApp->activeWindow());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(i18nc("@title:window", "Krita"));
        msgBox.setText(i18n("Krita cannot load the gmic-qt plugin. See below for more information."));
        msgBox.setDetailedText(loader.errorString());
        msgBox.exec();
        return;
    }

    plugin = qobject_cast<KritaGmicPluginInterface *>(loader.instance());
    if (!plugin) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Krita cannot launch the gmic-qt plugin. The provided library is not a valid plugin."));
        return;
    }

    m_key = QUuid::createUuid().toString();
    auto image = std::make_shared<KisImageInterface>(this->viewManager().data());
    int status = plugin->launch(image, again);

    dbgPlugins << "pluginFinished" << status;
    delete plugin;
    plugin = nullptr;
    loader.unload();

    m_qmicAction->setEnabled(true);
    m_againAction->setEnabled(true);
}

#include "QMic.moc"
