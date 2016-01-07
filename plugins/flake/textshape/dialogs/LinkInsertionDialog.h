/* This file is part of the KDE project
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
#ifndef LINKINSERTDIALOG
#define LINKINSERTDIALOG

#include <ui_LinkInsertionDialog.h>

#include <QDialog>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QWidget>

#include <KoBookmarkManager.h>
#include <KoTextRangeManager.h>
#include <KoTextDocument.h>
#include <QListWidget>
#define FETCH_TIMEOUT 5000

class LinkInsertionDialog : public QDialog
{
    Q_OBJECT
public :
    explicit LinkInsertionDialog(KoTextEditor *editor, QWidget *parent = 0);
    virtual ~LinkInsertionDialog();

private Q_SLOTS:
    void insertLink();

public Q_SLOTS:

    void fetchTitleFromURL();
    void replyFinished();
    void fetchTitleError(QNetworkReply::NetworkError);
    void updateTitleDownloadProgress(qint64, qint64);
    void fetchTitleTimeout();
    /**
    * Verifies the text entered in the four line edits : Weblink URL, Weblink text,
    * Bookmark name and Bookmark text. The "Ok" button is enabled only if the input
    * is valid.
    * @param text is the text to be verified.
    */
    void enableDisableButtons(QString text);

    /**
    * Once all the line edits for a tab have been verified, the OK button is enabled.
    * If the tab is switched, the validity of OK should be recalculated for the new tab.
    * @param text is the current active tab.
    */
    void checkInsertEnableValidity(int);

private :
    Ui::LinkInsertionDialog dlg;
    KoTextEditor *m_editor;
    const KoBookmarkManager *m_bookmarkManager;
    QStringList m_bookmarkList;
    QNetworkReply *m_reply;
    QNetworkAccessManager *m_networkAccessManager;
    QUrl m_linkURL;
    QTimer m_timeoutTimer;
    void accept();
    void sendRequest();
    void insertBookmarkLink(const QString &URL, const QString &text);
    void insertHyperlink(QString &linkURL, const QString &linkText);
    void displayInlineWarning(const QString &title, QLabel *label) const;
    bool exists(const QString &) const;
};
#endif
