
/* This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 * */

#include "KisResourceItemChooserContextMenu.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <KoResource.h>
#include <KisResourceModel.h>
#include <KisTagResourceModel.h>
#include <KisTag.h>


#include "kis_debug.h"

bool compareWithSpecialTags(KisTagSP tag) {
    // TODO: RESOURCES: id < 0? For now, "All" fits
    return !tag.isNull() && tag->id() < 0;
}


KisResourceItemChooserContextMenu::KisResourceItemChooserContextMenu(KoResourceSP resource,
                                                                     const KisTagSP currentlySelectedTag,
                                                                     KisTagChooserWidget *tagChooser)
    : m_tagChooserWidget(tagChooser)
{



    QImage image = resource->image();
    QIcon icon(QPixmap::fromImage(image));
    QAction *label = new QAction(resource->name(), this);
    label->setIcon(icon);

    addAction(label);

    QMenu *removableTagsMenu;
    QMenu *assignableTagsMenu;

    m_tagModel = new KisTagModel(resource->resourceType().first);
    KisResourceModel resourceModel(resource->resourceType().first);
    KisTagResourceModel tagResourceModel(resource->resourceType().first);

    tagResourceModel.setResourcesFilter(QVector<KoResourceSP>() << resource);
    QList<KisTagSP> removables;
    for (int i = 0; i < tagResourceModel.rowCount(); ++i) {
        removables << tagResourceModel.data(tagResourceModel.index(i, 0), Qt::UserRole + KisAllTagResourceModel::Tag).value<KisTagSP>();
    }


    QList<KisTagSP> list;
    for (int i = 0; i < m_tagModel->rowCount(); i++) {
        QModelIndex index = m_tagModel->index(i, 0);
        KisTagSP tag = m_tagModel->tagForIndex(index);
         if (!tag.isNull()) {
             list << tag;
         }
     }

    QList<KisTagSP> assignables2 = list;

    CompareWithOtherTagFunctor comparer(currentlySelectedTag);

    bool currentTagInRemovables = !currentlySelectedTag.isNull();
    currentTagInRemovables = currentTagInRemovables
            && (std::find_if(removables.begin(), removables.end(), comparer) != removables.end());

    // remove "All" tag from both "Remove from tag: " and "Assign to tag: " lists
    std::remove_if(removables.begin(), removables.end(), compareWithSpecialTags);
    std::remove_if(assignables2.begin(), assignables2.end(), compareWithSpecialTags);


    assignableTagsMenu = addMenu(koIcon("list-add"),i18n("Assign to tag"));


    if (!removables.isEmpty()) {
        addSeparator();
        KisTagSP currentTag = currentlySelectedTag;

        if (!currentTag.isNull() && currentTagInRemovables) {
            // remove the current tag from both "Remove from tag: " and "Assign to tag: " lists
            QList<QSharedPointer<KisTag> >::iterator b = std::remove_if(removables.begin(), removables.end(), comparer);
            if (b != removables.end()) {
                removables.removeAll(*b);
            }
            QList<QSharedPointer<KisTag> >::iterator b2 = std::remove_if(assignables2.begin(), assignables2.end(), comparer);
            if (b2 != assignables2.end()) {
                assignables2.removeAll(*b2);
            }

            SimpleExistingTagAction * removeTagAction = new SimpleExistingTagAction(resource, currentTag, this);
            removeTagAction->setText(i18n("Remove from this tag"));
            removeTagAction->setIcon(koIcon("list-remove"));

            connect(removeTagAction, SIGNAL(triggered(const KisTagSP, KoResourceSP)),
                    this, SLOT(removeResourceExistingTag(const KisTagSP, KoResourceSP)));
            addAction(removeTagAction);
        }

        if (!removables.isEmpty()) {
            removableTagsMenu = addMenu(koIcon("list-remove"),i18n("Remove from other tag"));

            KisTagSP empty;
            CompareWithOtherTagFunctor compareWithRemovable(empty);
            foreach (const KisTagSP tag, removables) {

                if (tag.isNull()) {
                    continue;
                }

                compareWithRemovable.setReferenceTag(tag);
                std::remove_if(assignables2.begin(), assignables2.end(), compareWithRemovable);

                SimpleExistingTagAction *removeTagAction = new SimpleExistingTagAction(resource, tag, this);

                connect(removeTagAction, SIGNAL(triggered(const KisTagSP, KoResourceSP)),
                        this, SLOT(removeResourceExistingTag(const KisTagSP, KoResourceSP)));
                removableTagsMenu->addAction(removeTagAction);
            }
        }

    }

    foreach (const KisTagSP &tag, assignables2) {
        if (tag.isNull()) {
            continue;
        }

        SimpleExistingTagAction * addTagAction = new SimpleExistingTagAction(resource, tag, this);

        connect(addTagAction, SIGNAL(triggered(const KisTagSP, KoResourceSP)),
                m_tagChooserWidget, SLOT(addTag(const KisTagSP, KoResourceSP)));

        assignableTagsMenu->addAction(addTagAction);
    }

    assignableTagsMenu->addSeparator();

    NewTagResourceAction *addNewTagAction = new NewTagResourceAction(resource, this);
    connect(addNewTagAction, SIGNAL(triggered(QString, KoResourceSP)), m_tagChooserWidget, SLOT(addTag(QString, KoResourceSP)));

    assignableTagsMenu->addAction(addNewTagAction);

}

KisResourceItemChooserContextMenu::~KisResourceItemChooserContextMenu()
{
    delete m_tagModel;
}

void KisResourceItemChooserContextMenu::removeResourceExistingTag(const KisTagSP tag, KoResourceSP resource)
{
    KisTagResourceModel tagResourceModel(resource->resourceType().first);
    tagResourceModel.untagResource(tag, resource);
}

