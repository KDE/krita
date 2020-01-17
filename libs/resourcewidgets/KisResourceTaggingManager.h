/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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
 *    Boston, MA 02110-1301, USA.
 */

#ifndef KISRESOURCETAGGINGMANAGER_H
#define KISRESOURCETAGGINGMANAGER_H

#include <QObject>

#include <KoResource.h>
#include <KisTag.h>
#include <KisTagModel.h>

#include <kis_debug.h>
#include <KisTag.h>

class QWidget;
class QStringList;
class QString;
class QPoint;

class KisTagFilterWidget;
class KisTagChooserWidget;
class KisTagFilterResourceProxyModel;


/**
 * @brief The KisResourceTaggingManager class is ...
 *
 * XXX: this needs to be documented!
 */
class KisResourceTaggingManager : public QObject
{
    Q_OBJECT

public:

    explicit KisResourceTaggingManager(QString resourceType, KisTagFilterResourceProxyModel *model, QWidget *parent);
    ~KisResourceTaggingManager() override;
    void showTaggingBar(bool show);
    void contextMenuRequested(KoResourceSP currentResource, QPoint pos);
    void allowTagModification( bool set );
    bool allowTagModification();
    KisTagFilterWidget *tagFilterWidget();
    KisTagChooserWidget *tagChooserWidget();
    
private Q_SLOTS:

    void tagSaveButtonPressed();
    void contextCreateNewTag(KoResourceSP resource, const KisTagSP tag);
    void contextRemoveTagFromResource(KoResourceSP resource, const KisTagSP tag);
    void contextAddTagToResource(KoResourceSP resource, const KisTagSP tag);
    void tagChooserIndexChanged(const KisTagSP lineEditText);
    void tagSearchLineEditTextChanged(const QString &lineEditText);

private:

    void enableContextMenu(bool enable);

    class Private;
    Private* const d;
};


#endif // KORESOURCETAGGINGINTERFACE_H
