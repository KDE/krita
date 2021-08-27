/*
  *   This file is part of the KDE project
  *   SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
  *   SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
  *   SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
  *   SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
  *   SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
  *   SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
  *   SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
  *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceTaggingManager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <KoResource.h>

#include <KisResourceModel.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisTagModel.h>

#include "KisTagFilterWidget.h"
#include "KisTagChooserWidget.h"
#include "KisResourceItemChooserContextMenu.h"
#include "kis_debug.h"
#include "KisTag.h"

class KisResourceTaggingManager::Private
{
public:
    /// \brief tagChooser tag chooser widget (tags combobox + tag tool button with the popup)
    KisTagChooserWidget *tagChooser;

    /// \brief tagFilter resources filter widget (resources filter box + "filter by tag" checkbox)
    KisTagFilterWidget *tagFilter;

    /// \brief model main data model for resources in the item chooser that the Tagging Manager is taking care of
    QPointer<KisTagFilterResourceProxyModel> model;

    /// \brief tagModel main tag model for tags in the tags combobox
    KisTagModel *tagModel;

    QString resourceType;

};





KisResourceTaggingManager::KisResourceTaggingManager(QString resourceType, KisTagFilterResourceProxyModel *model, QWidget *parent)

    : QObject(parent)
    , d(new Private())
{
    d->model = model;
    d->resourceType = resourceType;
    d->tagModel = new KisTagModel(resourceType);
    d->tagChooser = new KisTagChooserWidget(d->tagModel, resourceType, parent);
    d->tagFilter = new KisTagFilterWidget(d->tagModel, parent);

    d->model->setFilterInCurrentTag(d->tagFilter->isFilterByTagChecked());

    connect(d->tagChooser, SIGNAL(sigTagChosen(KisTagSP)), this, SLOT(tagChooserIndexChanged(KisTagSP)));

    connect(d->tagFilter, SIGNAL(filterByTagChanged(bool)), this, SLOT(slotFilterByTagChanged(bool)));
    connect(d->tagFilter, SIGNAL(filterTextChanged(QString)), this, SLOT(tagSearchLineEditTextChanged(QString)));

    connect(d->tagChooser, SIGNAL(sigTagChosen(KisTagSP)), d->tagFilter, SLOT(clear()));
}

KisResourceTaggingManager::~KisResourceTaggingManager()
{
    delete d->tagModel;
    delete d;
}

void KisResourceTaggingManager::showTaggingBar(bool show)
{
    show ? d->tagFilter->show() : d->tagFilter->hide();
    show ? d->tagChooser->show() : d->tagChooser->hide();

    KConfigGroup group =  KSharedConfig::openConfig()->group("SelectedTags");
    QString tag = group.readEntry<QString>(d->resourceType, "All");
    d->tagChooser->setCurrentItem(tag);
}

void KisResourceTaggingManager::tagChooserIndexChanged(const KisTagSP tag)
{
    d->model->setTagFilter(tag);
}

void KisResourceTaggingManager::tagSearchLineEditTextChanged(const QString& lineEditText)
{
    d->model->setSearchText(lineEditText);
}

void KisResourceTaggingManager::slotFilterByTagChanged(const bool filterByTag)
{
    d->model->setFilterInCurrentTag(filterByTag);
}

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP resource, QPoint pos)
{
    // No visible tag chooser usually means no intended tag interaction,
    // context menu makes no sense then either
    if (!resource || !d->tagChooser->isVisible())
        return;

    KisResourceItemChooserContextMenu menu(resource, d->tagChooser->currentlySelectedTag(), d->tagChooser);
    menu.exec(pos);

}

KisTagChooserWidget *KisResourceTaggingManager::tagChooserWidget()
{
    return d->tagChooser;
}

KisTagFilterWidget *KisResourceTaggingManager::tagFilterWidget()
{
    return d->tagFilter;
}

