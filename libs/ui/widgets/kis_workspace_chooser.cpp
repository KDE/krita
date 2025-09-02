/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_workspace_chooser.h"

#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPushButton>
#include <QAction>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <KisKineticScroller.h>

#include <klocalizedstring.h>

#include <KisResourceItemChooser.h>
#include <KisResourceItemListView.h>
#include <KoResource.h>
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KoDockWidgetTitleBar.h>
#include <KisMainWindow.h>
#include <KisTagFilterResourceProxyModel.h>

#include "kis_workspace_resource.h"
#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"
#include "KisMainWindow.h"
#include "KisPart.h"
#include "KisWindowLayoutManager.h"
#include "dialogs/KisNewWindowLayoutDialog.h"
#include "kis_config.h"
#include <kis_icon.h>
#include <KisResourceUserOperations.h>


class KisWorkspaceDelegate : public QStyledItemDelegate
{
public:
    static const int heightHint = 30;

public:
    KisWorkspaceDelegate(QObject * parent = 0) : QStyledItemDelegate(parent) {}
    ~KisWorkspaceDelegate() override {}


    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        if (!index.isValid())
            return;

        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Active : QPalette::Disabled;
        QPalette::ColorRole cr = (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text;
        painter->setPen(option.palette.color(cg, cr));

        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        else {
            painter->fillRect(option.rect, option.palette.base());
        }

        int verticalPadding = (option.rect.height() - painter->fontMetrics().ascent()) / 2;
        const int correction = -3;

        QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString();
        painter->drawText(
            option.rect.x() + 5,
            option.rect.y() + verticalPadding + painter->fontMetrics().ascent() + correction,
            name
        );
    }

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex &) const override
    {
        return QSize(-1, heightHint);
    }
};

KisWorkspaceChooser::KisWorkspaceChooser(KisViewManager* view, QWidget* parent): QWidget(parent), m_view(view)
{
    m_layout = new QGridLayout(this);

    // Workspaces
    m_workspaceWidgets = createChooserWidgets(ResourceType::Workspaces, i18n("Workspaces"));

    KisConfig cfg(true);
    QString workspaceName = cfg.readEntry<QString>("CurrentWorkspace");
    m_workspaceWidgets.itemChooser->setCurrentResource(workspaceName);

    connect(m_workspaceWidgets.itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
        this, SLOT(workspaceSelected(KoResourceSP )));
    connect(m_workspaceWidgets.saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveWorkspace()));
    connect(m_workspaceWidgets.nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotUpdateWorkspaceSaveButton()));

    slotUpdateWorkspaceSaveButton();

    // Window layouts
    m_windowLayoutWidgets = createChooserWidgets(ResourceType::WindowLayouts, i18n("Window layouts"));

    connect(m_windowLayoutWidgets.itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
        this, SLOT(windowLayoutSelected(KoResourceSP )));
    connect(m_windowLayoutWidgets.saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSaveWindowLayout()));
    connect(m_windowLayoutWidgets.nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotUpdateWindowLayoutSaveButton()));

    slotUpdateWindowLayoutSaveButton();
}

KisWorkspaceChooser::ChooserWidgets KisWorkspaceChooser::createChooserWidgets(const QString &resourceType, const QString &title)
{
    ChooserWidgets r;

    QLabel *titleLabel = new QLabel(this);
    QFont titleFont;
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setText(title);

    r.itemChooser = new KisResourceItemChooser(resourceType, false, this);
    r.itemChooser->setItemDelegate(new KisWorkspaceDelegate(this));
    r.itemChooser->setListViewMode(ListViewMode::Detail);
    r.itemChooser->setRowHeight(KisWorkspaceDelegate::heightHint);
    r.itemChooser->showTaggingBar(false);

    r.nameEdit = new QLineEdit(this);
    r.nameEdit->setPlaceholderText(i18n("Insert name"));
    r.nameEdit->setClearButtonEnabled(true);

    r.saveButton = new QPushButton(i18n("Save"));

    int firstRow = m_layout->rowCount();
    m_layout->addWidget(titleLabel,    firstRow,     0, 1, 2);
    m_layout->addWidget(r.itemChooser, firstRow + 1, 0, 1, 2);
    m_layout->addWidget(r.nameEdit,    firstRow + 2, 0, 1, 1);
    m_layout->addWidget(r.saveButton,  firstRow + 2, 1, 1, 1);

    return r;
}

KisWorkspaceChooser::~KisWorkspaceChooser()
{

}

void KisWorkspaceChooser::slotSaveWorkspace()
{
    if (!m_view->qtMainWindow()) {
        return;
    }

    KisWorkspaceResourceSP workspace;
    QString name = m_workspaceWidgets.nameEdit->text();
    if (name.isEmpty()) {
        return;
    }

    KisAllResourcesModel *resourceModel = KisResourceModelProvider::resourceModel(ResourceType::Workspaces);
    QVector<KoResourceSP> resources = resourceModel->resourcesForName(name);

    if (!resources.isEmpty()) {
        workspace = resources.first().dynamicCast<KisWorkspaceResource>();
    }

    if (workspace.isNull()) {
        workspace.reset(new KisWorkspaceResource(name));
        workspace->setName(name);
        workspace->setDockerState(m_view->qtMainWindow()->saveState());
        workspace->setImage(m_view->mainWindow()->layoutThumbnail());
        workspace->setValid(true);
        workspace->setFilename(name.replace(" ", "_") + workspace->defaultFileExtension());
        m_view->canvasResourceProvider()->notifySavingWorkspace(workspace);
        KisResourceUserOperations::addResourceWithUserInput(this, workspace);
    }
    else {
        workspace->setDockerState(m_view->qtMainWindow()->saveState());
        workspace->setImage(m_view->mainWindow()->layoutThumbnail());
        m_view->canvasResourceProvider()->notifySavingWorkspace(workspace);
        KisResourceUserOperations::updateResourceWithUserInput(this, workspace);
    }

    m_workspaceWidgets.nameEdit->clear();
    slotUpdateWorkspaceSaveButton();
}

void KisWorkspaceChooser::slotUpdateWorkspaceSaveButton()
{
    if (m_workspaceWidgets.nameEdit->text().isEmpty()) {
        m_workspaceWidgets.saveButton->setEnabled(false);
        return;
    }
    m_workspaceWidgets.saveButton->setEnabled(true);

    KisAllResourcesModel *model = KisResourceModelProvider::resourceModel(ResourceType::Workspaces);
    QVector<KoResourceSP> resources = model->resourcesForName(m_workspaceWidgets.nameEdit->text());
    KoResourceSP res = resources.count() > 0 ? resources.first() : nullptr;
    if (res && res->active()) {
        m_workspaceWidgets.saveButton->setIcon(KisIconUtils::loadIcon("warning"));
        m_workspaceWidgets.saveButton->setToolTip(i18n("File name already in use. Saving will overwrite the original Workspace."));
        m_workspaceWidgets.saveButton->setText(i18n("Overwrite"));
    }
    else {
        m_workspaceWidgets.saveButton->setIcon(QIcon());
        m_workspaceWidgets.saveButton->setToolTip(i18n("Save current workspace."));
        m_workspaceWidgets.saveButton->setText(i18n("Save"));
    }
}

void KisWorkspaceChooser::workspaceSelected(KoResourceSP resource)
{
    if (!m_view->qtMainWindow()) {
        return;
    }

    KisConfig cfg(false);
    cfg.writeEntry("CurrentWorkspace", resource->name());
    KisWorkspaceResourceSP workspace = resource.dynamicCast<KisWorkspaceResource>();
    KisMainWindow *mainWindow = qobject_cast<KisMainWindow*>(m_view->qtMainWindow());
    mainWindow->restoreWorkspace(workspace);
}

void KisWorkspaceChooser::slotSaveWindowLayout()
{
    KisMainWindow *thisWindow = qobject_cast<KisMainWindow*>(m_view->qtMainWindow());
    if (!thisWindow) return;

    if (m_windowLayoutWidgets.nameEdit->text().isEmpty()) {
        return;
    }

    KisNewWindowLayoutDialog dlg;
    dlg.setName(m_windowLayoutWidgets.nameEdit->text());
    dlg.exec();

    if (dlg.result() != QDialog::Accepted) return;

    QString name = dlg.name();
    bool showImageInAllWindows = dlg.showImageInAllWindows();
    bool primaryWorkspaceFollowsFocus = dlg.primaryWorkspaceFollowsFocus();

    KisWindowLayoutResourceSP layout = KisWindowLayoutResource::fromCurrentWindows(name, KisPart::instance()->mainWindows(), showImageInAllWindows, primaryWorkspaceFollowsFocus, thisWindow);
    layout->setValid(true);

    KisWindowLayoutManager::instance()->setShowImageInAllWindowsEnabled(showImageInAllWindows);
    KisWindowLayoutManager::instance()->setPrimaryWorkspaceFollowsFocus(primaryWorkspaceFollowsFocus, thisWindow->id());

    if (name.isEmpty()) {
        name = i18n("Window Layout");
    }

    layout->setName(name);
    layout->setFilename(name.split(" ").join("_") + layout->defaultFileExtension());
    KisResourceUserOperations::addResourceWithUserInput(this, layout);

    slotUpdateWindowLayoutSaveButton();
}

void KisWorkspaceChooser::slotUpdateWindowLayoutSaveButton()
{
    if (m_windowLayoutWidgets.nameEdit->text().isEmpty()) {
        m_windowLayoutWidgets.saveButton->setEnabled(false);
        return;
    }
    m_windowLayoutWidgets.saveButton->setEnabled(true);

    KisAllResourcesModel *model = KisResourceModelProvider::resourceModel(ResourceType::WindowLayouts);
    QVector<KoResourceSP> resources = model->resourcesForName(m_windowLayoutWidgets.nameEdit->text());
    if (resources.isEmpty()) return;

    KoResourceSP res = resources.first();

    if (res && res->active()) {
        m_windowLayoutWidgets.saveButton->setIcon(KisIconUtils::loadIcon("warning"));
        m_windowLayoutWidgets.saveButton->setToolTip(i18n("File name already in use. Saving will overwrite the original window layout."));
    } else {
        m_windowLayoutWidgets.saveButton->setIcon(QIcon());
        m_windowLayoutWidgets.saveButton->setToolTip(i18n("Save current window layout."));
    }
}

void KisWorkspaceChooser::windowLayoutSelected(KoResourceSP resource)
{
    KisWindowLayoutResourceSP layout = resource.dynamicCast<KisWindowLayoutResource>();
    layout->applyLayout();
}

// WORKAROUND: setting the row height of the resource chooser gets overridden somehow
// so set it again here
void KisWorkspaceChooser::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_workspaceWidgets.itemChooser->setRowHeight(KisWorkspaceDelegate::heightHint);
    m_windowLayoutWidgets.itemChooser->setRowHeight(KisWorkspaceDelegate::heightHint);
}
