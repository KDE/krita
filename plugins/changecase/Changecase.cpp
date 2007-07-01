/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include "Changecase.h"

#include <QTextBlock>
#include <QTextDocument>
#include <QRadioButton>
#include <QVBoxLayout>

#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>

Changecase::Changecase()
{
}

void Changecase::finishedWord(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void Changecase::finishedParagraph(QTextDocument *document, int cursorPosition)
{
    Q_UNUSED(document);
    Q_UNUSED(cursorPosition);
}

void Changecase::checkSection(QTextDocument *document, int startPosition, int endPosition)
{
    m_cursor = QTextCursor(document);
    m_cursor.setPosition(startPosition);
    m_cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
    m_document = document;

    m_startPosition = startPosition;
    m_endPosition = endPosition;

    KDialog *dialog = new KDialog();
    dialog->setCaption("Changecase");
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);

    QWidget *widget = new QWidget(dialog);

    m_sentenceCaseRadio = new QRadioButton(i18n("Sentence case"));
    m_lowerCaseRadio = new QRadioButton(i18n("lowercase"));
    m_upperCaseRadio = new QRadioButton(i18n("UPPER CASE"));
    m_initialCapsRadio = new QRadioButton(i18n("Initial Caps"));
    m_toggleCaseRadio = new QRadioButton(i18n("tOGGLE cASE"));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_sentenceCaseRadio);
    vLayout->addWidget(m_lowerCaseRadio);
    vLayout->addWidget(m_upperCaseRadio);
    vLayout->addWidget(m_initialCapsRadio);
    vLayout->addWidget(m_toggleCaseRadio);

    widget->setLayout(vLayout);
    dialog->setMainWidget(widget);

    dialog->show();

    connect(dialog, SIGNAL(accepted()), this, SLOT(process()));
}

void Changecase::process()
{
    if (m_sentenceCaseRadio->isChecked())
        sentenceCase();
    else if (m_lowerCaseRadio->isChecked())
        lowerCase();
    else if (m_upperCaseRadio->isChecked())
        upperCase();
    else if (m_initialCapsRadio->isChecked())
        initialCaps();
    else if (m_toggleCaseRadio->isChecked())
        toggleCase();
}

void Changecase::sentenceCase()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    QTextCursor backCursor(m_cursor);
    int pos = block.position() + block.length() - 1;
    QChar replacedChar;
    
    // TODO
    // * Exception?
    while (true) {
        QString text = block.text();
        int prevLetterIndex = -1;
        QChar currentWord;
        pos = block.position() + block.length() - 1;

        QString::Iterator iter = text.end();

        if (text.isEmpty()) { // empty block, go to next block
            if (!(block.isValid() && block.position() + block.length() < m_endPosition))
                break;
            block = block.next();
            continue;
        }

        iter--;
        while (iter != text.begin()) {
            while (iter != text.begin() && !iter->isSpace()) {
                iter--;
                pos--;
            }

            prevLetterIndex = pos;
            currentWord = QChar(*(iter + 1));
            while (iter != text.begin() && iter->isSpace()) {
                iter--;
                pos--;
            }

            if (iter != text.begin() && (*iter == QChar('.') || *iter == QChar('!') || *iter == QChar('?'))) {
                if (prevLetterIndex >= m_startPosition && prevLetterIndex <= m_endPosition) {
                    // kDebug() << "Found end of sentence " << *iter << " : " << currentWord << endl;
                    m_cursor.setPosition(prevLetterIndex);
                    m_cursor.deleteChar();
                    m_cursor.insertText(currentWord.toUpper());
                    iter--;
                    pos--;
                }
                else if (prevLetterIndex < m_startPosition)
                    break;
            }
        }

        if (iter == text.begin() && --pos >= m_startPosition) { // start of paragraph, must be start of a sentence also
            if (pos + 1 == prevLetterIndex) {
                m_cursor.setPosition(pos);
                m_cursor.deleteChar();
                m_cursor.insertText((*iter).toUpper());
            }
            else {
                m_cursor.setPosition(prevLetterIndex);
                m_cursor.deleteChar();
                m_cursor.insertText(currentWord.toUpper());
            }
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            break;
        block = block.next();
    }

    restoreSelection();
}

void Changecase::lowerCase()
{
    // TODO: do for each text block instead of whole selection? clean up the code
    QString text = m_cursor.selectedText();
    QString result;

    QString::ConstIterator constIter = text.constBegin();

    while (constIter != text.constEnd())
        result.append(constIter++->toLower());

    if(text != result)
        m_cursor.insertText(result);

    restoreSelection();
}

void Changecase::upperCase()
{
    QString text = m_cursor.selectedText();
    QString result;

    QString::ConstIterator constIter = text.constBegin();

    while (constIter != text.constEnd())
        result.append(constIter++->toUpper());

    if(text != result)
        m_cursor.insertText(result);

    restoreSelection();
}

void Changecase::initialCaps()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    int pos = block.position();
    bool finished = false;

    while (true) {
        QString text = block.text();
        QString result;
        bool space = true;

        QString::ConstIterator constIter = text.constBegin();
        while (pos < m_endPosition && constIter != text.constEnd()) {
            bool isSpace = constIter->isSpace();
            
            if (pos >= m_startPosition) {
                if (space && !isSpace)
                    result.append(constIter->toTitleCase());
                else
                    result.append(*constIter);
            }

            space = isSpace;
            pos++;
            constIter++;
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            finished = true;

        m_cursor.setPosition(qMax(m_startPosition, block.position()));
        m_cursor.setPosition(qMin(pos, m_endPosition), QTextCursor::KeepAnchor);
        m_cursor.insertText(result);

        if (finished)
            break;

        block = block.next();
        pos = block.position();
    }

    restoreSelection();
}

void Changecase::toggleCase()
{
    QString text = m_cursor.selectedText();
    QString result;

    QString::ConstIterator constIter = text.constBegin();

    while (constIter != text.constEnd()) {
        if (constIter->isLower())
            result.append(constIter->toUpper());
        else if (constIter->isUpper())
            result.append(constIter->toLower());
        else
            result.append(*constIter);
        constIter++;
    }

    if(text != result)
        m_cursor.insertText(result);

    restoreSelection();
}

void Changecase::restoreSelection()
{
    m_cursor.setPosition(m_startPosition);
    m_cursor.setPosition(m_endPosition, QTextCursor::KeepAnchor);
}

#include "Changecase.moc"
