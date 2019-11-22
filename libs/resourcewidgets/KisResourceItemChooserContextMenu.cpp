
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

#include "KisResourceItemChooserContextMenu.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <KoResource.h>

#include <KisTag.h>

#include "kis_debug.h"

KoLineEditAction::KoLineEditAction(QObject* parent)
    : QWidgetAction(parent)
    , m_closeParentOnTrigger(false)
{
    QWidget* pWidget = new QWidget (0);
    QHBoxLayout* pLayout = new QHBoxLayout();
    m_label = new QLabel(0);
    m_editBox = new QLineEdit(0);
    m_editBox->setClearButtonEnabled(true);
    m_AddButton = new QPushButton();
    m_AddButton->setIcon(koIcon("list-add"));
    pLayout->addWidget(m_label);
    pLayout->addWidget(m_editBox);
    pLayout->addWidget(m_AddButton);
    pWidget->setLayout(pLayout);
    setDefaultWidget(pWidget);

    connect (m_editBox, &QLineEdit::returnPressed, this, &KoLineEditAction::onTriggered);
    connect (m_AddButton, &QPushButton::clicked, this, &KoLineEditAction::onTriggered);
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
    ENTER_FUNCTION();
    if (! m_editBox->text().isEmpty()) {
        // TODO: RESOURCES: what about "remove existing tag" action?
        KisTagSP tag(new KisTag());
        tag->setName(m_editBox->text());
        tag->setUrl(m_editBox->text());
        emit triggered(tag);
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
    ENTER_FUNCTION();
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

ContextMenuExistingTagAction::ContextMenuExistingTagAction(KoResourceSP resource, KisTagSP tag, QObject* parent)
    : QAction(parent)
    , m_resource(resource)
    , m_tag(tag)
{
    setText(tag->name());
    connect (this, SIGNAL(triggered()),
             this, SLOT(onTriggered()));
}

ContextMenuExistingTagAction::~ContextMenuExistingTagAction()
{
}

void ContextMenuExistingTagAction::onTriggered()
{
    ENTER_FUNCTION();
    emit triggered(m_resource, m_tag);
}
NewTagAction::~NewTagAction()
{
}

NewTagAction::NewTagAction(KoResourceSP resource, QMenu* parent)
    :KoLineEditAction (parent)
{
    m_resource = resource;
    setIcon(koIcon("document-new"));
    setPlaceholderText(i18n("New tag"));
    closeParentOnTrigger(true);

    connect (this, SIGNAL(triggered(KisTagSP)),
             this, SLOT(onTriggered(KisTagSP)));
}

void NewTagAction::onTriggered(const KisTagSP tag)
{
    ENTER_FUNCTION();
    emit triggered(m_resource,tag);
}

KisResourceItemChooserContextMenu::KisResourceItemChooserContextMenu(KoResourceSP resource,
                                                                   const QList<KisTagSP> resourceTags,
                                                                   const KisTagSP currentlySelectedTag,
                                                                   const QList<KisTagSP> allTags)
{

    QImage image = resource->image();
    QIcon icon(QPixmap::fromImage(image));
    QAction * label = new QAction(resource->name(), this);
    label->setIcon(icon);

    addAction(label);

    QMenu * removableTagsMenu;
    QMenu * assignableTagsMenu;

    QList<KisTagSP> removables = resourceTags;
    QList<KisTagSP> assignables = allTags;


    std::sort(removables.begin(), removables.end(), KisTag::compareNamesAndUrls);
    std::sort(assignables.begin(), assignables.end(), KisTag::compareNamesAndUrls);


    assignableTagsMenu = addMenu(koIcon("list-add"),i18n("Assign to tag"));


    if (!removables.isEmpty()) {
        addSeparator();
        KisTagSP currentTag = currentlySelectedTag;

        if (!currentTag.isNull() && removables.contains(currentTag)) {
            //assignables.remove(currentTag);
            //removables.remove(currentTag);
            ContextMenuExistingTagAction * removeTagAction = new ContextMenuExistingTagAction(resource, currentTag, this);
            removeTagAction->setText(i18n("Remove from this tag"));
            removeTagAction->setIcon(koIcon("list-remove"));

            connect(removeTagAction, SIGNAL(triggered(KoResourceSP,KisTagSP)),
                    this, SIGNAL(resourceTagRemovalRequested(KoResourceSP,KisTagSP)));
            addAction(removeTagAction);
        }

        if (!removables.isEmpty()) {
            removableTagsMenu = addMenu(koIcon("list-remove"),i18n("Remove from other tag"));
            foreach (const KisTagSP tag, removables) {
                //assignables.remove(tag);
                if (tag.isNull()) {
                    continue;
                }

                ContextMenuExistingTagAction * removeTagAction = new ContextMenuExistingTagAction(resource, tag, this);

                connect(removeTagAction, SIGNAL(triggered(KoResourceSP,KisTagSP)),
                        this, SIGNAL(resourceTagRemovalRequested(KoResourceSP,KisTagSP)));
                removableTagsMenu->addAction(removeTagAction);
            }
        }

    }


    foreach (const KisTagSP &tag, assignables) {
        if (tag.isNull()) {
            continue;
        }

        ContextMenuExistingTagAction * addTagAction = new ContextMenuExistingTagAction(resource, tag, this);

        connect(addTagAction, SIGNAL(triggered(KoResourceSP, KisTagSP)),
                this, SIGNAL(resourceTagAdditionRequested(KoResourceSP, KisTagSP)));
        assignableTagsMenu->addAction(addTagAction);
    }
    /*
    assignableTagsMenu->addSeparator();

    NewTagAction * addTagAction = new NewTagAction(resource, this);
    connect(addTagAction, SIGNAL(triggered(KoResourceSP, KisTagSP)),
            this, SIGNAL(resourceAssignmentToNewTagRequested(KoResourceSP, KisTagSP)));
    assignableTagsMenu->addAction(addTagAction);
    */
}

KisResourceItemChooserContextMenu::~KisResourceItemChooserContextMenu()
{

}
