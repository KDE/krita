/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
