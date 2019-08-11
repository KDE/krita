/* This file is part of the KDE project
 * Copyright (C) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_workspace_chooser.h"

#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QPushButton>
#include <QAction>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <KisKineticScroller.h>

#include <klocalizedstring.h>

#include <KoResourceItemChooser.h>
#include <KoResourceItemView.h>
#include <KoResourceServerAdapter.h>
#include <resources/KoResource.h>

#include "KisResourceServerProvider.h"
#include "kis_workspace_resource.h"
#include "KisViewManager.h"
#include <kis_canvas_resource_provider.h>
#include <KisMainWindow.h>
#include <KisPart.h>
#include <KisWindowLayoutManager.h>
#include <dialogs/KisNewWindowLayoutDialog.h>
#include <kis_config.h>

class KisWorkspaceDelegate : public QAbstractItemDelegate
{
public:
    KisWorkspaceDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    ~KisWorkspaceDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }
};

void KisWorkspaceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (!index.isValid())
        return;

    KoResource* workspace = static_cast<KoResource*>(index.internalPointer());

    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Active : QPalette::Disabled;
    QPalette::ColorRole cr = (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text;
    painter->setPen(option.palette.color(cg, cr));

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    else {
        painter->fillRect(option.rect, option.palette.base());
    }

    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, workspace->name());
}

KisWorkspaceChooser::KisWorkspaceChooser(KisViewManager * view, QWidget* parent): QWidget(parent), m_view(view)
{
    KoResourceServer<KisWorkspaceResource> * workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    QSharedPointer<KoAbstractResourceServerAdapter> workspaceAdapter(new KoResourceServerAdapter<KisWorkspaceResource>(workspaceServer));

    KoResourceServer<KisWindowLayoutResource> * windowLayoutServer = KisResourceServerProvider::instance()->windowLayoutServer();
    QSharedPointer<KoAbstractResourceServerAdapter> windowLayoutAdapter(new KoResourceServerAdapter<KisWindowLayoutResource>(windowLayoutServer));

    m_layout = new QGridLayout(this);

    m_workspaceWidgets = createChooserWidgets(workspaceAdapter, i18n("Workspaces"));
    m_windowLayoutWidgets = createChooserWidgets(windowLayoutAdapter, i18n("Window layouts"));

    connect(m_workspaceWidgets.itemChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(workspaceSelected(KoResource*)));
    connect(m_workspaceWidgets.saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveWorkspace()));

    connect(m_windowLayoutWidgets.itemChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(windowLayoutSelected(KoResource*)));
    connect(m_windowLayoutWidgets.saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveWindowLayout()));
}

KisWorkspaceChooser::ChooserWidgets KisWorkspaceChooser::createChooserWidgets(QSharedPointer<KoAbstractResourceServerAdapter> adapter, const QString &title)
{
    ChooserWidgets widgets;

    QLabel *titleLabel = new QLabel(this);
    QFont titleFont;
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setText(title);

    widgets.itemChooser = new KoResourceItemChooser(adapter, this);
    widgets.itemChooser->setItemDelegate(new KisWorkspaceDelegate(this));
    widgets.itemChooser->setFixedSize(250, 250);
    widgets.itemChooser->setRowHeight(30);
    widgets.itemChooser->setColumnCount(1);
    widgets.itemChooser->showTaggingBar(false);
    widgets.saveButton = new QPushButton(i18n("Save"));

    widgets.nameEdit = new QLineEdit(this);
    widgets.nameEdit->setPlaceholderText(i18n("Insert name"));
    widgets.nameEdit->setClearButtonEnabled(true);

    int firstRow = m_layout->rowCount();
    m_layout->addWidget(titleLabel, firstRow, 0, 1, 2);
    m_layout->addWidget(widgets.itemChooser, firstRow + 1, 0, 1, 2);
    m_layout->addWidget(widgets.nameEdit, firstRow + 2, 0, 1, 1);
    m_layout->addWidget(widgets.saveButton, firstRow + 2, 1, 1, 1);

    return widgets;
}

KisWorkspaceChooser::~KisWorkspaceChooser()
{

}

void KisWorkspaceChooser::slotSaveWorkspace()
{
    if (!m_view->qtMainWindow()) {
        return;
    }

    KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();

    KisWorkspaceResource* workspace = new KisWorkspaceResource(QString());
    workspace->setDockerState(m_view->qtMainWindow()->saveState());
    m_view->canvasResourceProvider()->notifySavingWorkspace(workspace);
    workspace->setValid(true);
    QString saveLocation = rserver->saveLocation();
    QString name = m_workspaceWidgets.nameEdit->text();

    bool newName = false;
    if(name.isEmpty()) {
        newName = true;
        name = i18n("Workspace");
    }
    QFileInfo fileInfo(saveLocation + name + workspace->defaultFileExtension());

    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(saveLocation + name + QString("%1").arg(i) + workspace->defaultFileExtension());
        i++;
    }
    workspace->setFilename(fileInfo.filePath());
    if(newName) {
        name = i18n("Workspace %1", i);
    }
    workspace->setName(name);
    rserver->addResource(workspace);
}

void KisWorkspaceChooser::workspaceSelected(KoResource *resource)
{
    if (!m_view->qtMainWindow()) {
        return;
    }
    KisConfig cfg(false);
    cfg.writeEntry("CurrentWorkspace", resource->name());
    KisWorkspaceResource* workspace = static_cast<KisWorkspaceResource*>(resource);
    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(m_view->qtMainWindow());
    mainWindow->restoreWorkspace(workspace);

}

void KisWorkspaceChooser::slotSaveWindowLayout()
{
    KisMainWindow *thisWindow = qobject_cast<KisMainWindow*>(m_view->qtMainWindow());
    if (!thisWindow) return;

    KisNewWindowLayoutDialog dlg;
    dlg.setName(m_windowLayoutWidgets.nameEdit->text());
    dlg.exec();

    if (dlg.result() != QDialog::Accepted) return;

    QString name = dlg.name();
    bool showImageInAllWindows = dlg.showImageInAllWindows();
    bool primaryWorkspaceFollowsFocus = dlg.primaryWorkspaceFollowsFocus();

    auto *layout = KisWindowLayoutResource::fromCurrentWindows(name, KisPart::instance()->mainWindows(), showImageInAllWindows, primaryWorkspaceFollowsFocus, thisWindow);
    layout->setValid(true);

    KisWindowLayoutManager::instance()->setShowImageInAllWindowsEnabled(showImageInAllWindows);
    KisWindowLayoutManager::instance()->setPrimaryWorkspaceFollowsFocus(primaryWorkspaceFollowsFocus, thisWindow->id());

    KoResourceServer<KisWindowLayoutResource> * rserver = KisResourceServerProvider::instance()->windowLayoutServer();
    QString saveLocation = rserver->saveLocation();

    bool newName = false;
    if (name.isEmpty()) {
        newName = true;
        name = i18n("Window Layout");
    }
    QFileInfo fileInfo(saveLocation + name + layout->defaultFileExtension());

    int i = 1;
    while (fileInfo.exists()) {
        fileInfo.setFile(saveLocation + name + QString("%1").arg(i) + layout->defaultFileExtension());
        i++;
    }
    layout->setFilename(fileInfo.filePath());

    if (newName) {
        name = i18n("Window Layout %1", i);
    }
    layout->setName(name);
    rserver->addResource(layout);
}

void KisWorkspaceChooser::windowLayoutSelected(KoResource * resource)
{
    auto *layout = static_cast<KisWindowLayoutResource*>(resource);
    layout->applyLayout();
}
