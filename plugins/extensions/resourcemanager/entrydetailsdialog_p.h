/*
 *  Copyright (c) 2017 Aniketh Girish anikethgireesh@gmail.com
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

#ifndef ENTRYDETAILSDIALOG_P_H
#define ENTRYDETAILSDIALOG_P_H

#include <QtCore/QObject>

#include <KNSCore/EntryInternal>

#include "ui_wdgdlgcontentdownloader.h"

class QListWidgetItem;

namespace Ui {
class WdgDlgContentDownloader;
}

namespace KNSCore
{
class Engine;
}

class EntryDetails : public QObject
{
    Q_OBJECT
public:
    EntryDetails(KNSCore::Engine *engine, Ui::WdgDlgContentDownloader *widget);
    ~EntryDetails();

public Q_SLOTS:
    void setEntry(const KNSCore::EntryInternal &entry);

private Q_SLOTS:
    void slotEntryPreviewLoaded(const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::PreviewType type);
    void install();
    void uninstall();

    void ratingChanged(uint rating);
//    void becomeFan();
    // more details loaded
    void entryChanged(const KNSCore::EntryInternal &entry);
    // installed/updateable etc
    void entryStatusChanged(const KNSCore::EntryInternal &entry);
    void updateButtons();

    void preview1Selected();
    void preview2Selected();
    void preview3Selected();

private:
    void init();
    void previewSelected(int current);

    KNSCore::Engine *m_engine;
    Ui::WdgDlgContentDownloader *ui;
    KNSCore::EntryInternal m_entry;
    QImage m_currentPreview;
    QListWidgetItem *m_previewItem1;
    QListWidgetItem *m_previewItem2;
    QListWidgetItem *m_previewItem3;
};

#endif // ENTRYDETAILSDIALOG_P_H
