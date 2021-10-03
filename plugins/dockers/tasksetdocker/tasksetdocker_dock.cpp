/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "tasksetdocker_dock.h"

#include <QGridLayout>
#include <QListView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QInputDialog>
#include <QAction>
#include <QMessageBox>

#include <klocalizedstring.h>
#include <kactioncollection.h>

#include <kis_icon.h>

#include <KoCanvasBase.h>
#include <KisResourceItemChooser.h>
#include <KisResourceLoader.h>
#include <KisResourceItemListView.h>
#include <KisResourceLoaderRegistry.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <KisMainWindow.h>
#include <KisResourceUserOperations.h>

#include "tasksetmodel.h"


class KisTasksetDelegate : public QStyledItemDelegate
{
public:
    KisTasksetDelegate(QObject * parent = 0) : QStyledItemDelegate(parent) {}
    ~KisTasksetDelegate() override {}
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const override {
        return QSize(QStyledItemDelegate::sizeHint(option, index).width(),
                     qMin(QStyledItemDelegate::sizeHint(option, index).width(), 25));
    }
};

class KisTasksetResourceDelegate : public QStyledItemDelegate
{
public:
    KisTasksetResourceDelegate(QObject * parent = 0) : QStyledItemDelegate(parent) {}
    ~KisTasksetResourceDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
};

void KisTasksetResourceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (! index.isValid())
        return;

    QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }
    else {
        painter->setPen(option.palette.text().color());
    }
    int margin = option.rect.height() - painter->fontMetrics().height();
    margin = floor(margin/2);
    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + margin, name);
}

TasksetDockerDock::TasksetDockerDock( )
    : QDockWidget(i18n("Task Sets"))
    , m_canvas(0)
    , m_blocked(false)
{
    QWidget* widget = new QWidget(this);
    setupUi(widget);
    m_model = new TasksetModel(this);
    tasksetView->setModel(m_model);
    tasksetView->setItemDelegate(new KisTasksetDelegate(this));
    recordButton->setIcon(KisIconUtils::loadIcon("media-record"));
    recordButton->setCheckable(true);
    recordButton->setFlat(true);

    clearButton->setIcon(KisIconUtils::loadIcon("edit-delete"));
    clearButton->setFlat(true);

    saveButton->setIcon(KisIconUtils::loadIcon("document-save-16"));
    saveButton->setEnabled(false);
    saveButton->setFlat(true);

    chooserButton->setIcon(KisIconUtils::loadIcon("folder"));
    chooserButton->setAutoRaise(true);

    m_rserver = new KoResourceServer<TasksetResource>(ResourceType::TaskSets);
    KisResourceItemChooser *itemChooser = new KisResourceItemChooser(ResourceType::TaskSets, false, this);
    itemChooser->setItemDelegate(new KisTasksetResourceDelegate(this));
    itemChooser->setFixedSize(500, 250);
    itemChooser->setRowHeight(30);
    itemChooser->itemView()->setViewMode(QListView::ListMode);
    itemChooser->showTaggingBar(true);
    chooserButton->setPopupWidget(itemChooser);

    connect(itemChooser, SIGNAL(resourceSelected(KoResourceSP )), this, SLOT(resourceSelected(KoResourceSP )));

    setWidget(widget);

    connect( tasksetView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(activated(QModelIndex)) );

    connect( recordButton, SIGNAL(toggled(bool)), this, SLOT(recordClicked()));
    connect( clearButton, SIGNAL(clicked(bool)), this, SLOT(clearClicked()));
    connect( saveButton, SIGNAL(clicked(bool)), this, SLOT(saveClicked()));
}

TasksetDockerDock::~TasksetDockerDock()
{
    delete m_rserver;
}

void TasksetDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas && m_canvas->viewManager()) {
         m_canvas->viewManager()->actionCollection()->disconnect(this);
         Q_FOREACH (KXMLGUIClient* client, m_canvas->viewManager()->mainWindow()->childClients()) {
            client->actionCollection()->disconnect(this);
        }
    }
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
}

void TasksetDockerDock::unsetCanvas()
{
    m_canvas = 0;
    m_model->clear();
}

void TasksetDockerDock::actionTriggered(QAction* action)
{
    if(action && !action->objectName().isEmpty() &&
       !m_blocked && recordButton->isChecked()) {
        m_model->addAction(action);
        saveButton->setEnabled(true);
    }
}

void TasksetDockerDock::activated(const QModelIndex& index)
{
    QAction* action = m_model->actionFromIndex(index);
    m_blocked = true;
    if (action && action->isEnabled()) {
        action->trigger();
    }
    m_blocked = false;
}

void TasksetDockerDock::recordClicked()
{
    if(m_canvas) {
        KisViewManager* view = m_canvas->viewManager();
        connect(view->actionCollection(), SIGNAL(actionTriggered(QAction*)),
                this, SLOT(actionTriggered(QAction*)), Qt::UniqueConnection);
        Q_FOREACH (KXMLGUIClient* client, view->mainWindow()->childClients()) {
            connect(client->actionCollection(), SIGNAL(actionTriggered(QAction*)),
                    this, SLOT(actionTriggered(QAction*)), Qt::UniqueConnection);
        }
    }
}

void TasksetDockerDock::saveClicked()
{
    QString name;

    TasksetResourceSP taskset(new TasksetResource(QString()));

    QStringList actionNames;
    Q_FOREACH (QAction* action, m_model->actions()) {
        actionNames.append(action->objectName());
    }
    taskset->setActionList(actionNames);
    taskset->setValid(true);
    QString saveLocation = m_rserver->saveLocation();

    if (name.isEmpty()) {
        name = i18n("Taskset");
    }
    QFileInfo fileInfo(saveLocation + name + taskset->defaultFileExtension());

    bool fileOverwriteAccepted = false;
    bool ok = false;

    while(!fileOverwriteAccepted) {
        name = QInputDialog::getText(this, i18n("Taskset Name"),
                                     i18n("Name:"), QLineEdit::Normal,
                                     QString(), &ok);
        if (name.isNull() || name.isEmpty()) {
            return;
        } else {
            fileInfo = QFileInfo(saveLocation + name.split(" ").join("_") + taskset->defaultFileExtension());
            if (fileInfo.exists()) {
                int res = QMessageBox::warning(this, i18nc("@title:window", "Name Already Exists")
                                                        , i18n("The name '%1' already exists, do you wish to overwrite it?", name)
                                                        , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (res == QMessageBox::Yes) fileOverwriteAccepted = true;
            } else {
                fileOverwriteAccepted = true;
            }
        }
    }

    taskset->setName(name);
    taskset->setFilename(fileInfo.fileName());
    KisResourceModel model(ResourceType::TaskSets);
    KisResourceUserOperations::addResourceWithUserInput(this, &model, taskset);
}

void TasksetDockerDock::clearClicked()
{
    saveButton->setEnabled(false);
    m_model->clear();
}

void TasksetDockerDock::resourceSelected(KoResourceSP resource)
{
    if(!m_canvas) {
        return;
    }
    m_model->clear();
    saveButton->setEnabled(true);
    Q_FOREACH (const QString& actionName, resource.staticCast<TasksetResource>()->actionList()) {
        QAction* action = m_canvas->viewManager()->actionCollection()->action(actionName);
        if(action) {
            m_model->addAction(action);
        }
    }
}

static void addResourceLoader()
{
    KisResourceLoaderRegistry::instance()->registerLoader(new KisResourceLoader<TasksetResource>(ResourceType::TaskSets, ResourceType::TaskSets, i18n("Task sets"), QStringList() << "application/x-krita-taskset"));
}

Q_COREAPP_STARTUP_FUNCTION(addResourceLoader)
