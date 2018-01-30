/*
 *    This file is part of the KDE project
 *    Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
 *    Boston, MA 02110-1301, USA.
 */

#ifndef KOTAGCHOOSERWIDGET_H
#define KOTAGCHOOSERWIDGET_H

#include <QWidget>
#include "kritawidgets_export.h"

class KRITAWIDGETS_EXPORT KoTagChooserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KoTagChooserWidget(QWidget* parent);
    ~KoTagChooserWidget() override;
    void setCurrentIndex(int index);
    int findIndexOf(QString tagName);
    void insertItem(QString tagName);
    QString currentlySelectedTag();
    QStringList allTags();
    bool selectedTagIsReadOnly();
    void removeItem(QString item);
    void addItems(QStringList tagNames);
    void addReadOnlyItem(QString tagName);
    void clear();
    void setUndeletionCandidate(const QString &tag);
    void showTagToolButton(bool show);

Q_SIGNALS:
    void newTagRequested(const QString &tagname);
    void tagDeletionRequested(const QString &tagname);
    void tagRenamingRequested(const QString &oldTagname, const QString &newTagname);
    void tagUndeletionRequested(const QString &tagname);
    void tagUndeletionListPurgeRequested();
    void popupMenuAboutToShow();
    void tagChosen(const QString &tag);

private Q_SLOTS:
    void tagRenamingRequested(const QString &newName);
    void tagOptionsContextMenuAboutToShow();
    void contextDeleteCurrentTag();

private:
    /// pimpl because chooser will most likely get upgraded at some point
    class Private;
    Private* const d;

};
;

#endif // KOTAGCHOOSERWIDGET_H
