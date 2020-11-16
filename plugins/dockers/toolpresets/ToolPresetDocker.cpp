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

#include "ToolPresetDocker.h"

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
#include <kis_categorized_list_view.h>
#include <kis_categorized_item_delegate.h>

#include "ToolPresetModel.h"


ToolPresetDocker::ToolPresetDocker()
    : QDockWidget(i18n("Tool Option Presets"))
{
    setFeatures(DockWidgetMovable|DockWidgetFloatable);

    QWidget *page = new QWidget(this);
    setupUi(page);
    setWidget(page);

    bnAdd->setIcon(koIcon("document-new"));
    bnDelete->setIcon(koIcon("edit-delete"));

    m_toolPresetModel = new ToolPresetFilterProxyModel(0);
    lstPresets->setModel(m_toolPresetModel);
    lstPresets->setItemDelegate(new KisCategorizedItemDelegate(this));

    connect(KoToolManager::instance(), SIGNAL(toolOptionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)), this, SLOT(optionWidgetsChanged(KoCanvasController*,QList<QPointer<QWidget> >)));
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController *, int)), this, SLOT(toolChanged(KoCanvasController *, int)));
    connect(bnAdd, SIGNAL(clicked()), SLOT(bnAddPressed()));
    connect(bnDelete, SIGNAL(clicked()), SLOT(bnDeletePressed()));
    connect(lstPresets, SIGNAL(clicked(QModelIndex)), SLOT(presetSelected(QModelIndex)));
}

ToolPresetDocker::~ToolPresetDocker()
{
    delete m_toolPresetModel;
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
    qDebug() << "ToolPresetDocker::toolChanged";

    m_currentToolId = KoToolManager::instance()->activeToolId();

    if (chkExecuteToolOnSelection->isChecked()) {
        m_toolPresetModel->setFilter(m_currentToolId);
    }
    else {
        m_toolPresetModel->setFilter(QString());
    }

}

void ToolPresetDocker::bnAddPressed()
{
//    if (txtName->text().isEmpty()) {
//        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Please enter a name for the tool preset."));
//        return;
//    }

//    QString section = txtName->text();
//    QString optionFile = createConfigFileName(m_currentToolId);
//    Q_FOREACH (QPointer<QWidget> widget, m_currentOptionWidgets) {
//        if (widget) {
//            KisDialogStateSaver::saveState(widget, section, optionFile);
//        }
//    }

//    KoToolBase *tool = KoToolManager::instance()->toolById(m_canvas, m_currentToolId);
//    if (tool && tool->inherits("KisToolPaint")) {
//        KConfig cfg(createConfigFileName(m_currentToolId));
//        KConfigGroup grp = cfg.group(txtName->text());
//        KisPaintOpSettingsSP settings = m_resourceProvider->currentPreset()->settings();
//        grp.writeEntry("brush_size",settings->paintOpSize());
//    }
//    QListWidgetItem *item = new QListWidgetItem(toolIcon(m_currentToolId), section);
//    lstPresets->addItem(item);
//    lstPresets->blockSignals(true);
//    lstPresets->setCurrentItem(item);
//    lstPresets->blockSignals(false);
}

void ToolPresetDocker::bnDeletePressed()
{
//    if (!lstPresets->count()) return;

//    KConfig cfg(createConfigFileName(m_currentToolId), KConfig::SimpleConfig);
//    cfg.deleteGroup(lstPresets->currentItem()->text());
//    txtName->clear();
    //    delete lstPresets->takeItem(lstPresets->currentRow());
}

void ToolPresetDocker::presetSelected(const QModelIndex *idx)
{
    ToolPresetInfo *info = m_toolPresetModel->toolPresetInfo(idx->row());

    if (info->toolId != m_currentToolId) {
        KoToolManager::instance()->switchToolRequested(info->toolId);
    }

//    QString text = info->presetName;
//    txtName->setText(text);

//    Q_FOREACH (QPointer<QWidget> widget, m_currentOptionWidgets) {
//        if (widget) {
//            KisDialogStateSaver::restoreState(widget, text, QMap<QString, QVariant>(), createConfigFileName(m_currentToolId));
//        }
//    }

//    KConfig cfg(createConfigFileName(m_currentToolId));
//    KConfigGroup grp = cfg.group(text);

//    chkExecuteToolOnSelection->setChecked(grp.readEntry("execute_on_select", false));
//    chkSaveBrushPresetInformation->setChecked(grp.readEntry("save_resources", false));

    KoToolBase *tool = KoToolManager::instance()->toolById(m_canvas, m_currentToolId);
    if (tool && tool->inherits("KisToolPaint")) {
        m_resourceProvider->setSize(grp.readEntry("brush_size", m_resourceProvider->size()));
    }

}
