
/* This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  * Boston, MA 02110-1301, USA.
 * */

#include "KoResourceItemChooserContextMenu.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <qlineedit.h>

#include <resources/KoResource.h>

KoLineEditAction::KoLineEditAction(QObject* parent)
    : QWidgetAction(parent)
    , m_closeParentOnTrigger(false)
{
    QWidget* pWidget = new QWidget (0);
    QHBoxLayout* pLayout = new QHBoxLayout();
    m_label = new QLabel(0);
    m_editBox = new QLineEdit(0);
    pLayout->addWidget(m_label);
    pLayout->addWidget(m_editBox);
    pWidget->setLayout(pLayout);
    setDefaultWidget(pWidget);

    connect (m_editBox, SIGNAL(returnPressed()),
             this, SLOT(onTriggered()));
}

KoLineEditAction::~KoLineEditAction()
{

}

void KoLineEditAction::setIcon(const QIcon &icon)
{
    QPixmap pixmap = QPixmap(icon.pixmap(16,16));
    m_label->setPixmap(pixmap);
}

void KoLineEditAction::closeParentOnTrigger(bool closeParent)
{
    m_closeParentOnTrigger = closeParent;
}

bool KoLineEditAction::closeParentOnTrigger()
{
    return m_closeParentOnTrigger;
}

void KoLineEditAction::onTriggered()
{
    if (! m_editBox->text().isEmpty()) {
        emit triggered( m_editBox->text());
        m_editBox->text().clear();

        if (m_closeParentOnTrigger) {
            this->parentWidget()->close();
            m_editBox->clearFocus();
        }
    }
}

void KoLineEditAction::setPlaceholderText(const QString& clickMessage)
{
    m_editBox->setPlaceholderText(clickMessage);
}

void KoLineEditAction::setText(const QString& text)
{
    m_editBox->setText(text);
}

void KoLineEditAction::setVisible(bool showAction)
{
    QLayout* currentLayout = defaultWidget()->layout();

    this->QAction::setVisible(showAction);

    for(int i=0;i<currentLayout->count();i++) {
        currentLayout->itemAt(i)->widget()->setVisible(showAction);
    }
    defaultWidget()->setVisible(showAction);
}

ContextMenuExistingTagAction::ContextMenuExistingTagAction(KoResource* resource, QString tag, QObject* parent)
    : QAction(parent)
    , m_resource(resource)
    , m_tag(tag)
{
    setText(tag);
    connect (this, SIGNAL(triggered()),
             this, SLOT(onTriggered()));
}

ContextMenuExistingTagAction::~ContextMenuExistingTagAction()
{
}

void ContextMenuExistingTagAction::onTriggered()
{
    emit triggered(m_resource, m_tag);
}
NewTagAction::~NewTagAction()
{
}

NewTagAction::NewTagAction(KoResource* resource, QMenu* parent)
    :KoLineEditAction (parent)
{
    m_resource = resource;
    setIcon(koIcon("document-new"));
    setPlaceholderText(i18n("New tag"));
    closeParentOnTrigger(true);

    connect (this, SIGNAL(triggered(QString)),
             this, SLOT(onTriggered(QString)));
}

void NewTagAction::onTriggered(const QString & tagName)
{
    emit triggered(m_resource,tagName);
}

KoResourceItemChooserContextMenu::KoResourceItemChooserContextMenu(KoResource* resource,
                                                                   const QStringList& resourceTags,
                                                                   const QString& currentlySelectedTag,
                                                                   const QStringList& allTags)
{
    QImage image = resource->image();
    QIcon icon(QPixmap::fromImage(image));
    QAction * label = new QAction(resource->name(), this);
    label->setIcon(icon);

    addAction(label);

    QMenu * removableTagsMenu;
    QMenu * assignableTagsMenu;

    QStringList removables = resourceTags;
    QStringList assignables = allTags;

    removables.sort();
    assignables.sort();

    assignableTagsMenu = addMenu(koIcon("list-add"),i18n("Assign to tag"));

    if (!removables.isEmpty()) {
        addSeparator();
        QString currentTag = currentlySelectedTag;
        if (removables.contains(currentTag)) {
            assignables.removeAll(currentTag);
            removables.removeAll(currentTag);
            ContextMenuExistingTagAction * removeTagAction = new ContextMenuExistingTagAction(resource, currentTag, this);
            removeTagAction->setText(i18n("Remove from this tag"));
            removeTagAction->setIcon(koIcon("list-remove"));

            connect(removeTagAction, SIGNAL(triggered(KoResource*,QString)),
                    this, SIGNAL(resourceTagRemovalRequested(KoResource*,QString)));
            addAction(removeTagAction);
        }
        if (!removables.isEmpty()) {
            removableTagsMenu = addMenu(koIcon("list-remove"),i18n("Remove from other tag"));
            foreach (const QString &tag, removables) {
                assignables.removeAll(tag);
                ContextMenuExistingTagAction * removeTagAction = new ContextMenuExistingTagAction(resource, tag, this);

                connect(removeTagAction, SIGNAL(triggered(KoResource*,QString)),
                        this, SIGNAL(resourceTagRemovalRequested(KoResource*,QString)));
                removableTagsMenu->addAction(removeTagAction);
            }
        }
    }

    foreach (const QString &tag, assignables) {
        ContextMenuExistingTagAction * addTagAction = new ContextMenuExistingTagAction(resource, tag, this);

        connect(addTagAction, SIGNAL(triggered(KoResource*,QString)),
                this, SIGNAL(resourceTagAdditionRequested(KoResource*,QString)));
        assignableTagsMenu->addAction(addTagAction);
    }
    assignableTagsMenu->addSeparator();

    NewTagAction * addTagAction = new NewTagAction(resource, this);
    connect(addTagAction, SIGNAL(triggered(KoResource*,QString)),
            this, SIGNAL(resourceAssignmentToNewTagRequested(KoResource*,QString)));
    assignableTagsMenu->addAction(addTagAction);
}

KoResourceItemChooserContextMenu::~KoResourceItemChooserContextMenu()
{

}
