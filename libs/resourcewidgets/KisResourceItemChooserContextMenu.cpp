
/* This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
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
#include <KisTagModelProvider.h>
#include <KisTag.h>


#include "kis_debug.h"

bool compareWithSpecialTags(KisTagSP tag) {
    // TODO: RESOURCES: id < 0? For now, "All" fits
    return !tag.isNull() && tag->id() < 0;
}


KisResourceItemChooserContextMenu::KisResourceItemChooserContextMenu(KoResourceSP resource,
                                                                     const KisTagSP currentlySelectedTag)
{

    QImage image = resource->image();
    QIcon icon(QPixmap::fromImage(image));
    QAction *label = new QAction(resource->name(), this);
    label->setIcon(icon);

    addAction(label);

    QMenu *removableTagsMenu;
    QMenu *assignableTagsMenu;

    m_tagModel = KisTagModelProvider::tagModel(resource->resourceType().first);

    QList<KisTagSP> removables = m_tagModel->tagsForResource(resource->resourceId()).toList();

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

    ENTER_FUNCTION() << "current tag in removeables: " <<currentTagInRemovables;


    // remove "All" tag from both "Remove from tag: " and "Assign to tag: " lists
    std::remove_if(removables.begin(), removables.end(), compareWithSpecialTags);
    std::remove_if(assignables2.begin(), assignables2.end(), compareWithSpecialTags);


    assignableTagsMenu = addMenu(koIcon("list-add"),i18n("Assign to tag"));


    if (!removables.isEmpty()) {
        addSeparator();
        KisTagSP currentTag = currentlySelectedTag;

        if (!currentTag.isNull() && currentTagInRemovables) {
            // remove the current tag from both "Remove from tag: " and "Assign to tag: " lists
            ENTER_FUNCTION() << "# remove the current tag from both lists";

            ENTER_FUNCTION() << "now just removeables";
            ENTER_FUNCTION() << "comparer's tag: " << comparer.referenceTag();
            QList<QSharedPointer<KisTag> >::iterator b = std::remove_if(removables.begin(), removables.end(), comparer);
            if (b != removables.end()) {
                removables.removeAll(*b);
            }
            QList<QSharedPointer<KisTag> >::iterator b2 = std::remove_if(assignables2.begin(), assignables2.end(), comparer);
            if (b2 != assignables2.end()) {
                assignables2.removeAll(*b2);
            }
            ENTER_FUNCTION() << "done. The list now consists of: ";
            Q_FOREACH(KisTagSP tag, removables) {
                ENTER_FUNCTION() << tag;
            }
            ENTER_FUNCTION() << "end";

            ExistingTagAction * removeTagAction = new ExistingTagAction(resource, currentTag, this);
            removeTagAction->setText(i18n("Remove from this tag"));
            removeTagAction->setIcon(koIcon("list-remove"));

            connect(removeTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
                    this, SLOT(removeResourceExistingTag(KoResourceSP, const KisTagSP)));
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

                ExistingTagAction * removeTagAction = new ExistingTagAction(resource, tag, this);

                connect(removeTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
                        this, SLOT(removeResourceExistingTag(KoResourceSP, const KisTagSP)));
                removableTagsMenu->addAction(removeTagAction);
            }
        }

    }


    foreach (const KisTagSP &tag, assignables2) {
        if (tag.isNull()) {
            continue;
        }

        ExistingTagAction * addTagAction = new ExistingTagAction(resource, tag, this);

        connect(addTagAction, SIGNAL(triggered(KoResourceSP, const KisTagSP)),
                this, SLOT(addResourceExistingTag(KoResourceSP, const KisTagSP)));

        assignableTagsMenu->addAction(addTagAction);
    }

    assignableTagsMenu->addSeparator();

    NewTagAction *addNewTagAction = new NewTagAction(resource, this);
    connect(addNewTagAction, SIGNAL(triggered(KoResourceSP, QString)), this, SLOT(addResourceNewTag(KoResourceSP, QString)));
    assignableTagsMenu->addAction(addNewTagAction);

}

KisResourceItemChooserContextMenu::~KisResourceItemChooserContextMenu()
{
}

void KisResourceItemChooserContextMenu::addResourceExistingTag(KoResourceSP resource, const KisTagSP tag)
{
    m_tagModel->tagResource(tag, resource);
}

void KisResourceItemChooserContextMenu::removeResourceExistingTag(KoResourceSP resource, const KisTagSP tag)
{
    m_tagModel->untagResource(tag, resource);
}

void KisResourceItemChooserContextMenu::addResourceNewTag(KoResourceSP resource, const QString tag)
{
    qDebug() << "addResourceNewTag" << tag << resource;
    m_tagModel->addEmptyTag(tag, {resource});
}
