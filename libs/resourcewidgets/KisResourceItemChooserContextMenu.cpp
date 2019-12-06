
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
    fprintf(stderr, "created a new KoLineEditAction\n");
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

    fprintf(stderr, "void KoLineEditAction::onTriggered()");

    if (! m_editBox->text().isEmpty()) {
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
    fprintf(stderr, "ContextMenuExistingTagAction created for: %s\n", tag->name().toUtf8().toStdString().c_str());
    setText(tag->name());
    connect (this, SIGNAL(triggered()),
             this, SLOT(onTriggered()));
}

ContextMenuExistingTagAction::~ContextMenuExistingTagAction()
{
}

void ContextMenuExistingTagAction::onTriggered()
{
    fprintf(stderr, "void ContextMenuExistingTagAction::onTriggered()\n");
    ENTER_FUNCTION();
    emit triggered(m_resource, m_tag);
}
NewTagAction::~NewTagAction()
{
}

NewTagAction::NewTagAction(KoResourceSP resource, QMenu* parent)
    :KoLineEditAction (parent)
{
    fprintf(stderr, "NewTagAction created\n");
    m_resource = resource;
    setIcon(koIcon("document-new"));
    setPlaceholderText(i18n("New tag"));
    closeParentOnTrigger(true);

    connect (this, SIGNAL(triggered(KisTagSP)),
             this, SLOT(onTriggered(KisTagSP)));
}

void NewTagAction::onTriggered(const KisTagSP tag)
{
    emit triggered(m_resource,tag);
}

class CompareWithOtherTagFunctor
{
    KisTagSP m_referenceTag;

public:
    CompareWithOtherTagFunctor(KisTagSP referenceTag)
    {
        m_referenceTag = referenceTag;
    }

    bool operator()(KisTagSP otherTag)
    {
        return !otherTag.isNull() && otherTag->url() == m_referenceTag->url();
    }

};

bool compareWithSpecialTags(KisTagSP tag) {
    // TODO: RESOURCES: id < 0? For now, "All" fits
    return !tag.isNull() && tag->id() < 0;
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
    QList<KisTagSP> assignables2 = allTags;

    CompareWithOtherTagFunctor comparer(currentlySelectedTag);


    //std::sort(removables.begin(), removables.end(), KisTag::compareNamesAndUrls);
    //std::sort(assignables2.begin(), assignables2.end(), KisTag::compareNamesAndUrls);
    bool currentTagInRemovables = !currentlySelectedTag.isNull();
    currentTagInRemovables = currentTagInRemovables
            && (std::find_if(removables.begin(), removables.end(), comparer) != removables.end());


    // remove "All" tag from both "Remove from this tag" and "Assign to this tag" list
    std::remove_if(removables.begin(), removables.end(), compareWithSpecialTags);
    std::remove_if(assignables2.begin(), assignables2.end(), compareWithSpecialTags);


    assignableTagsMenu = addMenu(koIcon("list-add"),i18n("Assign to tag"));


    if (!removables.isEmpty()) {
        addSeparator();
        KisTagSP currentTag = currentlySelectedTag;

        if (!currentTag.isNull() && currentTagInRemovables) {
            std::remove_if(removables.begin(), removables.end(), comparer);
            std::remove_if(assignables2.begin(), assignables2.end(), comparer);

            ContextMenuExistingTagAction * removeTagAction = new ContextMenuExistingTagAction(resource, currentTag, this);
            removeTagAction->setText(i18n("Remove from this tag"));
            removeTagAction->setIcon(koIcon("list-remove"));

            connect(removeTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
                    this, SIGNAL(resourceTagRemovalRequested(KoResourceSP, const KisTagSP)));
            addAction(removeTagAction);
        }

        if (!removables.isEmpty()) {
            removableTagsMenu = addMenu(koIcon("list-remove"),i18n("Remove from other tag"));
            foreach (const KisTagSP tag, removables) {
                std::remove_if(assignables2.begin(), assignables2.end(), comparer);
                if (tag.isNull()) {
                    continue;
                }

                ContextMenuExistingTagAction * removeTagAction = new ContextMenuExistingTagAction(resource, tag, this);

                connect(removeTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
                        this, SIGNAL(resourceTagRemovalRequested(KoResourceSP, const KisTagSP)));
                removableTagsMenu->addAction(removeTagAction);
            }
        }

    }


    foreach (const KisTagSP &tag, assignables2) {
        if (tag.isNull()) {
            continue;
        }

        ContextMenuExistingTagAction * addTagAction = new ContextMenuExistingTagAction(resource, tag, this);

        connect(addTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
                this, SIGNAL(resourceTagAdditionRequested(KoResourceSP, const KisTagSP)));


        assignableTagsMenu->addAction(addTagAction);
    }

    assignableTagsMenu->addSeparator();

    NewTagAction * addTagAction = new NewTagAction(resource, this);
    connect(addTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
            this, SIGNAL(resourceAssignmentToNewTagRequested(KoResourceSP, const KisTagSP)));
    assignableTagsMenu->addAction(addTagAction);

}

KisResourceItemChooserContextMenu::~KisResourceItemChooserContextMenu()
{

}
