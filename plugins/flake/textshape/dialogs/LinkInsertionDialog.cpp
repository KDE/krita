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
 * Boston, MA 02110-1301, USA.q
 */
#include "LinkInsertionDialog.h"
#include "SimpleTableOfContentsWidget.h"

#include <KoTextEditor.h>

#include <QString>
#include <QCompleter>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>

LinkInsertionDialog::LinkInsertionDialog(KoTextEditor *editor, QWidget *parent)
    : QDialog(parent)
    , m_editor(editor)
    , m_bookmarkManager(0)
    , m_bookmarkList(0)
    , m_reply(0)
    , m_networkAccessManager(0)
    , m_linkURL(0)
    , m_timeoutTimer(0)
{
    dlg.setupUi(this);
    setUpdatesEnabled(false);
    // set up the tabs with selected text
    QString suggestedLinkText;
    if (m_editor->hasSelection()) {
        suggestedLinkText = m_editor->selectedText();
        dlg.hyperlinkText->setText(suggestedLinkText);
        dlg.bookmarkLinkText->setText(suggestedLinkText);
    }
    connect(dlg.buttonBox, SIGNAL(accepted()), this, SLOT(insertLink()));
    connect(dlg.buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ///setting up the web link insertion tab
    m_networkAccessManager = new QNetworkAccessManager(this);
    connect(dlg.fetchTitleButton, SIGNAL(clicked()), this, SLOT(fetchTitleFromURL()));
    dlg.fetchTitleButton->setEnabled(false);
    setUpdatesEnabled(true);

    ///setting up the bookmark link insertion tab
    //connect(dlg.bookmarkListWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemClicked(QListWidgetItem*)));
    m_bookmarkManager =  KoTextDocument(editor->document()).textRangeManager()->bookmarkManager();
    m_bookmarkList = m_bookmarkManager->bookmarkNameList();
    QCompleter *bookmarkAutoCompleter = new QCompleter(m_bookmarkList, this);
    dlg.bookmarkLinkURL->setCompleter(bookmarkAutoCompleter);
    dlg.bookmarkLinkURL->addItems(m_bookmarkList);
    dlg.bookmarkLinkURL->clearEditText();
    connect(dlg.hyperlinkURL, SIGNAL(textChanged(QString)), this, SLOT(enableDisableButtons(QString)));
    connect(dlg.hyperlinkText, SIGNAL(textChanged(QString)), this, SLOT(enableDisableButtons(QString)));
    connect(dlg.bookmarkLinkURL, SIGNAL(editTextChanged(QString)), this, SLOT(enableDisableButtons(QString)));
    connect(dlg.bookmarkLinkText, SIGNAL(textChanged(QString)), this, SLOT(enableDisableButtons(QString)));

    connect(dlg.linkTypesTab, SIGNAL(currentChanged(int)), this, SLOT(checkInsertEnableValidity(int)));
    show();
}
void LinkInsertionDialog::enableDisableButtons(QString text)
{
    text = text.trimmed();
    QObject *signalSender = (sender());
    if (qobject_cast< QLineEdit * >(signalSender) == dlg.hyperlinkURL) { //deal with fetch button
        if (!text.isEmpty()) { // is empty?
            if (!QUrl(text).isValid()) { //not empty, is it valid?
                dlg.fetchTitleButton->setEnabled(false);
                dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);//not valid too, time to get out
                displayInlineWarning(i18n("The URL is invalid"), dlg.weblinkStatusLabel);
                return;
            } else { //valid but non empty, can fetch but not sure about the others so can't OK
                displayInlineWarning("", dlg.weblinkStatusLabel);
                dlg.fetchTitleButton->setEnabled(true);
            }
        } else { //field is empty, no other choice
            dlg.fetchTitleButton->setEnabled(false);
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        }
    } else if (qobject_cast< QComboBox * >(signalSender) == dlg.bookmarkLinkURL) { //need to check existence
        if (dlg.bookmarkLinkURL->currentText().isEmpty()) {
            displayInlineWarning("", dlg.bookmarkLinkStatusLabel);
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        } else if (!exists(dlg.bookmarkLinkURL->currentText())) { //definitely can't go in
            displayInlineWarning(i18n("Bookmark does not exist"), dlg.bookmarkLinkStatusLabel);
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return;
        } else {  //non empty and exits
            displayInlineWarning("", dlg.bookmarkLinkStatusLabel); //can clear the label but cannot be sure about OK
        }
    } else if (text.isEmpty()) { //for others, empty is definitely incorrect
        dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }
    switch (dlg.linkTypesTab->currentIndex()) { //handle cases that reach here, only doubt is completeness
    case 0 :
        if (!dlg.hyperlinkText->text().isEmpty() && QUrl(dlg.hyperlinkURL->text()).isValid() && !dlg.hyperlinkURL->text().isEmpty()) {
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        break;
    case 1:
        if (!dlg.bookmarkLinkText->text().isEmpty() && !dlg.bookmarkLinkURL->currentText().isEmpty() && exists(dlg.bookmarkLinkURL->currentText())) {
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        break;
    }
}

void LinkInsertionDialog::checkInsertEnableValidity(int currentTab)
{
    dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    switch (currentTab) {
    case 0 :
        if (!dlg.hyperlinkText->text().isEmpty() && QUrl(dlg.hyperlinkURL->text()).isValid() && !dlg.hyperlinkURL->text().isEmpty()) {
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        break;
    case 1:
        if (!dlg.bookmarkLinkText->text().isEmpty() && !dlg.bookmarkLinkURL->currentText().isEmpty() && exists(dlg.bookmarkLinkURL->currentText())) {
            dlg.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
        break;
    }
}

void LinkInsertionDialog::insertLink()
{
    if (dlg.linkTypesTab->currentIndex() == 0) {
        QString linkText = dlg.hyperlinkText->text();
        QString linkURL = dlg.hyperlinkURL->text();
        insertHyperlink(linkURL, linkText);
    } else {
        QString linkName = dlg.bookmarkLinkURL->currentText();
        QString linkText = dlg.bookmarkLinkText->text();
        insertBookmarkLink(linkName, linkText);
    }
}

void LinkInsertionDialog::displayInlineWarning(const QString &warning, QLabel *label) const
{
    label->setText(warning);
}

void LinkInsertionDialog::insertHyperlink(QString &linkURLString, const QString &linkText)
{
    QString linkhtml;
    QUrl linkURL  = QUrl(linkURLString);
    dlg.weblinkStatusLabel->setText("");
    if (!linkURL.isValid()) {
        displayInlineWarning(i18n("The URL is invalid"), dlg.weblinkStatusLabel);
    } else {
        if ((linkURL.scheme()).isEmpty()) {  //prepend a scheme if not present
            linkURLString.prepend("http://");
        }
        m_editor->insertText(linkText, linkURLString);
        this->close();
    }
}

void LinkInsertionDialog::insertBookmarkLink(const QString &linkURL, const QString &linkText)
{
    dlg.bookmarkLinkStatusLabel->setText("");
    m_editor->insertText(linkText, linkURL);
    this->close();
}

bool LinkInsertionDialog::exists(const QString &bookmarkName) const
{
    return m_bookmarkList.contains(bookmarkName);
}

void LinkInsertionDialog::fetchTitleFromURL()
{
    QString linkURLString = dlg.hyperlinkURL->text();
    m_linkURL = QUrl(linkURLString);
    if (m_linkURL.isValid()) {
        if ((m_linkURL.scheme()).isEmpty()) {  //prepend a scheme if not present
            linkURLString.prepend("http://");
            dlg.hyperlinkURL->setText(linkURLString);
            m_linkURL.setUrl(linkURLString);
        }
        sendRequest();
    } else {
        displayInlineWarning(i18n("The URL is invalid"), dlg.weblinkStatusLabel);
        return;
    }
    //xgettext: no-c-format
    dlg.weblinkStatusLabel->setText(i18n("Fetching the title: 0% complete"));
}
void LinkInsertionDialog::sendRequest()
{
    QNetworkRequest request;
    request.setUrl(m_linkURL);
    m_reply = m_networkAccessManager->get(request);
    //start a timer to notify user when it takes too long to get the title
    if (m_timeoutTimer.isActive()) { //a timer for every redirection
        m_timeoutTimer.stop();
    }
    m_timeoutTimer.setInterval(FETCH_TIMEOUT);
    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.start();
    connect(&m_timeoutTimer, SIGNAL(timeout()), this, SLOT(fetchTitleTimeout()));
    connect(m_reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(fetchTitleError(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateTitleDownloadProgress(qint64,qint64)));
}

void LinkInsertionDialog::fetchTitleTimeout()
{
    if (!m_reply->isFinished()) {
        displayInlineWarning(i18n("Fetch timed out"), dlg.weblinkStatusLabel);
        m_reply->abort();
    }
}

void LinkInsertionDialog::replyFinished()
{
    //check for redirections
    QVariant possibleRedirectVariant =
        m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    QUrl possibleRedirectUrl = possibleRedirectVariant.toUrl();
    if (!possibleRedirectUrl.isEmpty() && m_linkURL != possibleRedirectUrl) { //redirect
        if (possibleRedirectUrl.toString().at(0) == '/') { //redirection to a relative url
            if (m_linkURL.toString().at(m_linkURL.toString().length() - 1) == '/') { //initially of the form http:xyz.com/
                possibleRedirectUrl.setUrl(m_linkURL.toString() + possibleRedirectUrl.toString().remove(0, 1));
            } else {
                possibleRedirectUrl.setUrl(m_linkURL.toString() + possibleRedirectUrl.toString());
            }
        }
        m_linkURL = possibleRedirectUrl;
        sendRequest();
        return;
    }
    const QString res = m_reply->readAll();
    static QRegExp titleStart("<title");//title tag can have attributes, so can't search for <title>
    static QRegExp titleEnd("</title>");
    int start = titleStart.indexIn(res);
    if (start == -1) { //perhaps TITLE?, rare but possible
        start = QRegExp("<TITLE").indexIn(res);
        if (start == -1) {
            displayInlineWarning("Error fetching title", dlg.weblinkStatusLabel);
            return;
        }
    }
    //now move to the end of the title
    while (res.at(start) != QChar('>')) {
        start++;
    }
    start++; //eat the '>'
    int end = titleEnd.indexIn(res);
    if (end == -1) {
        end = QRegExp("</TITLE>").indexIn(res);
        if (end == -1) {
            displayInlineWarning("Error fetching title", dlg.weblinkStatusLabel);
            return;
        }
    }
    dlg.hyperlinkText->setText(QStringRef(&res, start, end - start).toString());
    dlg.weblinkStatusLabel->setText("");
}

void LinkInsertionDialog::updateTitleDownloadProgress(qint64 received, qint64 total)
{
    float percentComplete = (static_cast<float>(received) / total) * 100;
    //xgettext: no-c-format
    dlg.weblinkStatusLabel->setText(i18n("Fetching the title: %1% complete", QString::number(percentComplete)));
}

LinkInsertionDialog::~LinkInsertionDialog()
{
    m_networkAccessManager->deleteLater();
}

void LinkInsertionDialog::fetchTitleError(QNetworkReply::NetworkError)
{
    m_timeoutTimer.stop();
    displayInlineWarning(i18n("The URL is invalid"), dlg.weblinkStatusLabel);
}

void LinkInsertionDialog::accept()
{
    //Overloaded to prevent the dialog from closing
}
