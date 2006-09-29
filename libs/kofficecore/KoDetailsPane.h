/* This file is part of the KDE project
   Copyright (C) 2005-2006 Peter Simonsson <psn@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KODETAILSPANE_H
#define KODETAILSPANE_H

#include "ui_koDetailsPaneBase.h"

class QEvent;
class KInstance;
class KUrl;
class QStandardItemModel;

class KoDetailsPanePrivate;

class KoDetailsPane : public QWidget, public Ui_KoDetailsPaneBase
{
  Q_OBJECT
  public:
    KoDetailsPane(QWidget* parent, KInstance* _instance, const QString& header);
    virtual ~KoDetailsPane();

    KInstance* instance();

    virtual bool eventFilter(QObject* watched, QEvent* e);

    /// @return the model used in the document list
    QStandardItemModel* model() const;

  signals:
    /// Emited when a file is requested to be opened
    void openUrl(const KUrl&);

    /// This is used to keep all splitters in different details panes synced
    void splitterResized(KoDetailsPane* sender, const QList<int>& sizes);

  public slots:
    /// This is used to keep all splitters in different details panes synced
    void resizeSplitter(KoDetailsPane* sender, const QList<int>& sizes);

  protected slots:
    /// This is called when the selection in the listview changed
    virtual void selectionChanged(const QModelIndex& index) = 0;
    virtual void openFile();
    virtual void openFile(const QModelIndex& index) = 0;

    void changePalette();

  private:
    KoDetailsPanePrivate* d;
};

#endif //KODETAILSPANE_H
