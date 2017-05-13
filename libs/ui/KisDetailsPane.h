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
#ifndef KISDETAILSPANE_H
#define KISDETAILSPANE_H

#include "ui_KisDetailsPaneBase.h"

class QEvent;
class QUrl;
class QStandardItemModel;

struct KisDetailsPanePrivate;

class KisDetailsPane : public QWidget, public Ui_KisDetailsPaneBase
{
    Q_OBJECT

public:
    KisDetailsPane(QWidget* parent, const QString& header);
    ~KisDetailsPane() override;

    bool eventFilter(QObject* watched, QEvent* e) override;

    /// @return the model used in the document list
    QStandardItemModel* model() const;

Q_SIGNALS:
    /// Emitted when a file is requested to be opened
    void openUrl(const QUrl&);

    /// This is used to keep all splitters in different details panes synced
    void splitterResized(KisDetailsPane* sender, const QList<int>& sizes);

public Q_SLOTS:
    /// This is used to keep all splitters in different details panes synced
    void resizeSplitter(KisDetailsPane* sender, const QList<int>& sizes);

protected Q_SLOTS:
    /// This is called when the selection in the listview changed
    virtual void selectionChanged(const QModelIndex& index) = 0;
    virtual void openFile();
    virtual void openFile(const QModelIndex& index) = 0;

    void changePalette();

protected:
    enum Extents {
        IconExtent = 64,
        PreviewExtent = 128
    };

private:
    KisDetailsPanePrivate * const d;
};

#endif //KODETAILSPANE_H
