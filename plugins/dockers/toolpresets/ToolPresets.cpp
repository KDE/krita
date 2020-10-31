/*
 *  Copyright (c) 2020 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ToolPresets.h"

#include <QApplication>
#include <QAction>
#include <QThread>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QStandardPaths>

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KoToolRegistry.h>
#include <kis_image_config.h>
#include <kis_icon.h>
#include <KoCanvasBase.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>
#include "kis_signal_compressor.h"

#include <kis_types.h>
#include <KisViewManager.h>
#include <KoToolManager.h>
#include <KoCanvasController.h>
#include <KoToolBase.h>
#include <KoToolFactoryBase.h>
#include <kis_canvas_resource_provider.h>
#include <KisDialogStateSaver.h>
#include <kis_paintop_preset.h>
#include <kis_paintop_settings.h>

#include "ToolPresets.h"

static QIcon toolIcon(const QString &toolId)
{
    KoToolFactoryBase *factory = KoToolRegistry::instance()->value(toolId);
    return koIcon(factory->iconName().toLatin1());
}

static QString createConfigFileName(QString toolId)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + "toolpresets/" + toolId.replace('/', '_') + ".toolpreset";
}

ToolPresetDocker::ToolPresetDocker()
    : QDockWidget(i18n("Tool Presets"))
{
    setFeatures(DockWidgetMovable|DockWidgetFloatable);

    QWidget *page = new QWidget(this);
    setupUi(page);
    setWidget(page);

    bnSave->setIcon(koIcon("document-save"));
    bnDelete->setIcon(koIcon("edit-delete"));

    connect(KoToolManager::instance(), SIGNAL(toolOptionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)), this, SLOT(optionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)));
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController *, int)), this, SLOT(toolChanged(KoCanvasController *, int)));
    connect(bnSave, SIGNAL(clicked()), SLOT(bnSavePressed()));
    connect(bnDelete, SIGNAL(clicked()), SLOT(bnDeletePressed()));
    connect(lstPresets, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(itemSelected(QListWidgetItem*)));
}

ToolPresetDocker::~ToolPresetDocker()
{

}

void ToolPresetDocker::setViewManager(KisViewManager *kisview)
{
    m_resourceProvider = kisview->canvasResourceProvider();
}

void ToolPresetDocker::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = canvas;
    setEnabled(canvas != 0);
}

void ToolPresetDocker::unsetCanvas()
{
    m_canvas = 0;
    setEnabled(false);
}

void ToolPresetDocker::optionWidgetsChanged(KoCanvasController */*canvasController*/, QList<QPointer<QWidget> > optionWidgets)
{

    m_currentOptionWidgets = optionWidgets;
}

void ToolPresetDocker::toolChanged(KoCanvasController *canvasController, int /*toolId*/)
{
    m_currentToolId = KoToolManager::instance()->activeToolId();

    lstPresets->clear();
    txtName->clear();

    KConfig cfg(createConfigFileName(m_currentToolId), KConfig::SimpleConfig);

    Q_FOREACH(const QString &group, cfg.groupList()) {
        QListWidgetItem *item = new QListWidgetItem(toolIcon(m_currentToolId), group, lstPresets);
    }
}

void ToolPresetDocker::bnSavePressed()
{
    if (txtName->text().isEmpty()) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Please enter a name for the tool preset."));
        return;
    }

    QString section = txtName->text();
    QString optionFile = createConfigFileName(m_currentToolId);
    Q_FOREACH (QPointer<QWidget> widget, m_currentOptionWidgets) {
        if (widget) {
            KisDialogStateSaver::saveState(widget, section, optionFile);
        }
    }

    KoToolBase *tool = KoToolManager::instance()->toolById(m_canvas, m_currentToolId);
    if (tool && tool->inherits("KisToolPaint")) {
        KConfig cfg(createConfigFileName(m_currentToolId));
        KConfigGroup grp = cfg.group(txtName->text());
        KisPaintOpSettingsSP settings = m_resourceProvider->currentPreset()->settings();
        grp.writeEntry("brush_size",settings->paintOpSize());
    }
    QListWidgetItem *item = new QListWidgetItem(toolIcon(m_currentToolId), section);
    lstPresets->addItem(item);
    lstPresets->blockSignals(true);
    lstPresets->setCurrentItem(item);
    lstPresets->blockSignals(false);
}

void ToolPresetDocker::bnDeletePressed()
{
    if (!lstPresets->count()) return;

    KConfig cfg(createConfigFileName(m_currentToolId), KConfig::SimpleConfig);
    cfg.deleteGroup(lstPresets->currentItem()->text());
    delete lstPresets->takeItem(lstPresets->currentRow());
}

void ToolPresetDocker::itemSelected(QListWidgetItem *item)
{
    Q_FOREACH (QPointer<QWidget> widget, m_currentOptionWidgets) {
        if (widget) {
            KisDialogStateSaver::restoreState(widget, item->text(), QMap<QString, QVariant>(), createConfigFileName(m_currentToolId));
        }
    }

    KoToolBase *tool = KoToolManager::instance()->toolById(m_canvas, m_currentToolId);
    if (tool && tool->inherits("KisToolPaint")) {
        KConfig cfg(createConfigFileName(m_currentToolId));
        KConfigGroup grp = cfg.group(item->text());
        m_resourceProvider->setSize(grp.readEntry("brush_size", m_resourceProvider->size()));
    }
}
