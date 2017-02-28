/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISDLGIMPORTIMAGESEQUENCE_H
#define KISDLGIMPORTIMAGESEQUENCE_H

#include <QObject>
#include <QCollator>
#include "KoDialog.h"
#include "ui_wdgimportimagesequence.h"

class KisDocument;
class KisMainWindow;

class KisDlgImportImageSequence : public KoDialog
{
    Q_OBJECT

public:
    KisDlgImportImageSequence(KisMainWindow *m_mainWindow, KisDocument *m_document);

    QStringList showOpenFileDialog();
    QStringList files();
    int firstFrame();
    int step();

protected Q_SLOTS:
    void slotAddFiles();
    void slotRemoveFiles();
    void slotSkipChanged(int);
    void slotOrderOptionsChanged(int);

private:
    void sortFileList();

private:
    Ui_WdgImportImageSequence m_ui;
    KisMainWindow *m_mainWindow;
    KisDocument *m_document;

    enum OrderingOptions {
        Ascending = 1,
        Descending = 2,
        Natural = 4,
        Numerical = 8
    };

    class ListItem;
    QCollator m_collator;
};

#endif // KISDLGIMPORTIMAGESEQUENCE_H
