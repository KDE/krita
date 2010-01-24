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
    dialog->setCaption(i18n("Change case"));
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
    emit startMacro(i18n("Change case"));

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

    emit stopMacro();
}

void Changecase::sentenceCase()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    QTextCursor backCursor(m_cursor);
    int pos = block.position() + block.length() - 1;

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

            // found end of sentence, go back to last found letter (indicating start of a word)
            if (iter != text.begin() && (*iter == QChar('.') || *iter == QChar('!') || *iter == QChar('?'))) {
                if (prevLetterIndex >= m_startPosition && prevLetterIndex <= m_endPosition && currentWord.isLower()) {
                    // kDebug() <<"Found end of sentence" << *iter <<" :" << currentWord;
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
            if (pos + 1 == prevLetterIndex && (*iter).isLower()) {
                m_cursor.setPosition(pos);
                m_cursor.deleteChar();
                m_cursor.insertText((*iter).toUpper());
            }
            else if (!(*iter).isLetter() && currentWord.isLower()) {
                m_cursor.setPosition(prevLetterIndex);
                m_cursor.deleteChar();
                m_cursor.insertText(currentWord.toUpper());
            }
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            break;
        block = block.next();
    }
}

void Changecase::lowerCase()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    int pos = block.position();
    bool finished = false;
    bool foundToBeChanged = false;

    while (true) {
        QString text = block.text();
        QString result;

        QString::ConstIterator constIter = text.constBegin();
        while (pos < m_endPosition && constIter != text.constEnd()) {
            if (pos >= m_startPosition) {
                if (!foundToBeChanged && constIter->isUpper())
                    foundToBeChanged = true;
                result.append(constIter->toLower());
            }

            pos++;
            constIter++;
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            finished = true;

        if (foundToBeChanged) {
            m_cursor.setPosition(qMax(m_startPosition, block.position()));
            m_cursor.setPosition(qMin(pos, m_endPosition), QTextCursor::KeepAnchor);
            m_cursor.insertText(result);
        }

        if (finished)
            break;

        block = block.next();
        pos = block.position();
    }
}

void Changecase::upperCase()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    int pos = block.position();
    bool finished = false;
    bool foundToBeChanged = false;

    while (true) {
        QString text = block.text();
        QString result;

        QString::ConstIterator constIter = text.constBegin();
        while (pos < m_endPosition && constIter != text.constEnd()) {
            if (pos >= m_startPosition) {
                if (!foundToBeChanged && constIter->isLower())
                    foundToBeChanged = true;
                result.append(constIter->toUpper());
            }

            pos++;
            constIter++;
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            finished = true;

        if (foundToBeChanged) {
            m_cursor.setPosition(qMax(m_startPosition, block.position()));
            m_cursor.setPosition(qMin(pos, m_endPosition), QTextCursor::KeepAnchor);
            m_cursor.insertText(result);
        }

        if (finished)
            break;

        block = block.next();
        pos = block.position();
    }
}

void Changecase::initialCaps()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    int pos = block.position();
    bool finished = false;
    bool foundToBeChanged = false;

    while (true) {
        QString text = block.text();
        QString result;
        bool space = true;

        QString::ConstIterator constIter = text.constBegin();
        while (pos < m_endPosition && constIter != text.constEnd()) {
            bool isSpace = constIter->isSpace();
            
            if (pos >= m_startPosition) {
                if (space && !isSpace) {
                    if (!foundToBeChanged && constIter->isLower())
                        foundToBeChanged = true;
                    result.append(constIter->toTitleCase());
                }
                else
                    result.append(*constIter);
            }

            space = isSpace;
            pos++;
            constIter++;
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            finished = true;

        if (foundToBeChanged) {
            m_cursor.setPosition(qMax(m_startPosition, block.position()));
            m_cursor.setPosition(qMin(pos, m_endPosition), QTextCursor::KeepAnchor);
            m_cursor.insertText(result);
        }

        if (finished)
            break;

        block = block.next();
        pos = block.position();
    }
}

void Changecase::toggleCase()
{
    QTextBlock block = m_document->findBlock(m_startPosition);
    int pos = block.position();
    bool finished = false;

    while (true) {
        QString text = block.text();
        QString result;

        QString::ConstIterator constIter = text.constBegin();
        while (pos < m_endPosition && constIter != text.constEnd()) {
            if (pos >= m_startPosition) {
                if (constIter->isLower())
                    result.append(constIter->toUpper());
                else if (constIter->isUpper())
                    result.append(constIter->toLower());
                else
                    result.append(*constIter);
            }

            pos++;
            constIter++;
        }

        if (!(block.isValid() && block.position() + block.length() < m_endPosition))
            finished = true;

        if (result != text) {
            m_cursor.setPosition(qMax(m_startPosition, block.position()));
            m_cursor.setPosition(qMin(pos, m_endPosition), QTextCursor::KeepAnchor);
            m_cursor.insertText(result);
        }

        if (finished)
            break;

        block = block.next();
        pos = block.position();
    }
}

#include <Changecase.moc>
