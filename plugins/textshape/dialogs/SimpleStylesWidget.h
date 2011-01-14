/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
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
#ifndef SIMPLESTYLESWIDGET_H
#define SIMPLESTYLESWIDGET_H

#include <QWidget>
#include <QTextBlockFormat>
#include <QTextCharFormat>


class StylesWidget;
class KoStyleManager;
class KoParagraphStyle;
class KoCharacterStyle;

class SimpleStylesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleStylesWidget(QWidget *parent = 0);

public slots:
    void setStyleManager(KoStyleManager *sm);
    void setCurrentFormat(const QTextBlockFormat &format);
    void setCurrentFormat(const QTextCharFormat &format);

signals:
    void doneWithFocus();
    void characterStyleSelected(KoCharacterStyle *);
    void paragraphStyleSelected(KoParagraphStyle *);

private:
    KoStyleManager *m_styleManager;
    QTextBlockFormat m_currentBlockFormat;
    QTextCharFormat m_currentCharFormat;
    bool m_blockSignals;
    StylesWidget *m_popupForBlock;
    StylesWidget *m_popupForChar;
};

#endif
