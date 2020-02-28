/*
  *   This file is part of the KDE project
  *   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
  *   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
  *   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
  *   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
  *   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
  *   Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
  *   Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
  *   This library is free software; you can redistribute it and/or
  *   modify it under the terms of the GNU Library General Public
  *   License as published by the Free Software Foundation; either
  *   version 2 of the License, or (at your option) any later version.
 *
  *   This library is distributed in the hope that it will be useful,
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  *   Library General Public License for more details.
 *
  *   You should have received a copy of the GNU Library General Public License
  *   along with this library; see the file COPYING.LIB.  If not, write to
  *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  *   Boston, MA 02110-1301, USA.
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
#include <KisResourceModelProvider.h>
#include <KisTagFilterResourceProxyModel.h>
#include <KisTagModel.h>
#include <KisTagModelProvider.h>

#include "KisTagFilterWidget.h"
#include "KisTagChooserWidget.h"
#include "KisResourceItemChooserContextMenu.h"
#include "kis_debug.h"
#include "KisTag.h"

class KisResourceTaggingManager::Private
{
public:
    KisTagChooserWidget *tagChooser;
    KisTagFilterWidget *tagFilter;

    QPointer<KisTagFilterResourceProxyModel> model;

    KisTagModel* tagModel;
};


KisResourceTaggingManager::KisResourceTaggingManager(QString resourceType, KisTagFilterResourceProxyModel *model, QWidget *parent)

    : QObject(parent)
    , d(new Private())
{
    d->model = model;

    d->tagModel = KisTagModelProvider::tagModel(resourceType);
    d->tagChooser = new KisTagChooserWidget(d->tagModel, parent);
    d->tagFilter = new KisTagFilterWidget(d->tagModel, parent);

    connect(d->tagChooser, SIGNAL(sigTagChosen(KisTagSP)), this, SLOT(tagChooserIndexChanged(KisTagSP)));

    connect(d->tagFilter, SIGNAL(filterByTagChanged(bool)), this, SLOT(slotFilterByTagChanged(bool)));
    connect(d->tagFilter, SIGNAL(filterTextChanged(QString)), this, SLOT(tagSearchLineEditTextChanged(QString)));
}

KisResourceTaggingManager::~KisResourceTaggingManager()
{
    delete d;
}

void KisResourceTaggingManager::showTaggingBar(bool show)
{
    show ? d->tagFilter->show() : d->tagFilter->hide();
    show ? d->tagChooser->show() : d->tagChooser->hide();
}

void KisResourceTaggingManager::tagChooserIndexChanged(const KisTagSP tag)
{
    d->model->setTag(tag);
    d->tagFilter->clear();
}

void KisResourceTaggingManager::tagSearchLineEditTextChanged(const QString& lineEditText)
{
    d->model->setSearchBoxText(lineEditText);
}

void KisResourceTaggingManager::slotFilterByTagChanged(const bool filterByTag)
{
    d->model->setFilterByCurrentTag(filterByTag);
}

void KisResourceTaggingManager::contextMenuRequested(KoResourceSP resource, QPoint pos)
{
    // No visible tag chooser usually means no intended tag interaction,
    // context menu makes no sense then either
    if (!resource || !d->tagChooser->isVisible())
        return;

    KisResourceItemChooserContextMenu menu(resource, d->tagChooser->currentlySelectedTag());
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

