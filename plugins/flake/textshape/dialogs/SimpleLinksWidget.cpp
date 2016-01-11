/* This file is part of the KDE project
 * Copyright (C) 2001 David Faure <faure@kde.org>
 * Copyright (C) 2005-2007, 2009, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010-2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2013 Aman Madaan <madaan.amanmadaan@gmail.com>
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

#include "SimpleLinksWidget.h"

#include "ReferencesTool.h"
#include <QAction>
#include <KoBookmarkManager.h>
#include "ManageBookmarkDialog.h"
#include <KoTextDocument.h>
#include <KoBookmark.h>
#include <KoTextRangeManager.h>
#include <KoTextRange.h>
#include <KoCanvasResourceManager.h>
#include <KoCanvasBase.h>

SimpleLinksWidget::SimpleLinksWidget(ReferencesTool *tool, QWidget *parent)
    : QWidget(parent)
    , m_referenceTool(tool)
{
    widget.setupUi(this);
    Q_ASSERT(tool);
    widget.insertLink->setDefaultAction(tool->action("insert_link"));
    widget.invokeBookmarkHandler->setDefaultAction(tool->action("invoke_bookmark_handler"));
    widget.invokeBookmarkHandler->setNumColumns(1);
    connect(widget.insertLink, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.invokeBookmarkHandler, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.invokeBookmarkHandler, SIGNAL(aboutToShowMenu()), this, SLOT(preparePopUpMenu()));
}

void SimpleLinksWidget::preparePopUpMenu()
{
    if (widget.invokeBookmarkHandler->isFirstTimeMenuShown()) {
        widget.invokeBookmarkHandler->addAction(m_referenceTool->action("insert_bookmark"));
        widget.invokeBookmarkHandler->addSeparator();
        widget.invokeBookmarkHandler->addAction(m_referenceTool->action("manage_bookmarks"));
        connect(m_referenceTool->action("manage_bookmarks"),
                SIGNAL(triggered()), this, SLOT(manageBookmarks()), Qt::UniqueConnection);
    }
}

void SimpleLinksWidget::manageBookmarks()
{
    QString name;
    const KoBookmarkManager *manager =  KoTextDocument(m_referenceTool->editor()->document()).textRangeManager()->bookmarkManager();
    QPointer<ManageBookmarkDialog> dia = new ManageBookmarkDialog(manager->bookmarkNameList(), m_referenceTool->editor(), m_referenceTool->canvas()->canvasWidget());
    connect(dia, SIGNAL(nameChanged(QString,QString)), manager, SLOT(rename(QString,QString)));
    connect(dia, SIGNAL(bookmarkDeleted(QString)), manager, SLOT(remove(QString)));
    if (dia->exec() == QDialog::Accepted) {
        name = dia->selectedBookmarkName();
    } else {
        delete dia;
        return;
    }
    delete dia;
    KoBookmark *bookmark = manager->bookmark(name);

    KoCanvasResourceManager *rm = m_referenceTool->canvas()->resourceManager();
    if ((bookmark->positionOnlyMode() == false) && bookmark->hasRange()) {
        rm->clearResource(KoText::SelectedTextPosition);
        rm->clearResource(KoText::SelectedTextAnchor);
    }
    if (bookmark->positionOnlyMode()) {
        rm->setResource(KoText::CurrentTextPosition, bookmark->rangeStart());
        rm->setResource(KoText::CurrentTextAnchor, bookmark->rangeStart());
    } else {
        rm->setResource(KoText::CurrentTextPosition, bookmark->rangeStart());
        rm->setResource(KoText::CurrentTextAnchor, bookmark->rangeEnd());
    }
}

SimpleLinksWidget::~SimpleLinksWidget()
{

}
