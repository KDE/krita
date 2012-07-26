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

#include <klocale.h>

#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>
#include <KoMainWindow.h>
#include <KoResource.h>

#include "kis_pattern.h"
#include "kis_resource_server_provider.h"
#include "kis_workspace_resource.h"
#include "kis_view2.h"
#include <QGridLayout>
#include <klineedit.h>
#include <kis_canvas_resource_provider.h>

class KisWorkspaceDelegate : public QAbstractItemDelegate
{
public:
    KisWorkspaceDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    virtual ~KisWorkspaceDelegate() {}
    /// reimplemented
    virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const {
        return option.decorationSize;
    }
};

void KisWorkspaceDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (! index.isValid())
        return;

    KisWorkspaceResource* workspace = static_cast<KisWorkspaceResource*>(index.internalPointer());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        painter->fillRect(option.rect, option.palette.highlight());
    }

    painter->setPen(Qt::black);
    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, workspace->name());      

}

KisWorkspaceChooser::KisWorkspaceChooser(KisView2 * view, QWidget* parent): QWidget(parent), m_view(view)
{
    KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();
    KoAbstractResourceServerAdapter* adapter = new KoResourceServerAdapter<KisWorkspaceResource>(rserver);
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setItemDelegate(new KisWorkspaceDelegate(this));
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->setRowHeight(30);
    m_itemChooser->setColumnCount(1);
    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(resourceSelected(KoResource*)));
    
    QPushButton* saveButton = new QPushButton(i18n("Save"));
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(slotSave()));
    
    m_nameEdit = new KLineEdit(this);
    m_nameEdit->setClickMessage(i18n("Insert name"));
    m_nameEdit->setClearButtonShown(true);
    
    QGridLayout* layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_itemChooser, 0, 0, 1, 2);
    layout->addWidget(m_nameEdit, 1, 0, 1, 1);
    layout->addWidget(saveButton, 1, 1, 1, 1);
}

KisWorkspaceChooser::~KisWorkspaceChooser()
{

}

void KisWorkspaceChooser::slotSave()
{
    if(!m_view->shell()) {
        return;
    }
    KoResourceServer<KisWorkspaceResource> * rserver = KisResourceServerProvider::instance()->workspaceServer();

    KisWorkspaceResource* workspace = new KisWorkspaceResource("");
    workspace->setDockerState(m_view->shell()->saveState());
    m_view->resourceProvider()->notifySavingWorkspace(workspace);
    workspace->setValid(true);
    QString saveLocation = rserver->saveLocation();
    QString name = m_nameEdit->text();
    
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
        name = i18n("Workspace %1").arg(i);
    }
    workspace->setName(name);
    rserver->addResource(workspace);
}

void KisWorkspaceChooser::resourceSelected(KoResource* resource)
{
    if(!m_view->shell()) {
        return;
    }
    KisWorkspaceResource* workspace = static_cast<KisWorkspaceResource*>(resource);
    m_view->shell()->restoreState(workspace->dockerState());
    m_view->resourceProvider()->notifyLoadingWorkspace(workspace);
}
