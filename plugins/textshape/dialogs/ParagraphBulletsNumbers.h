/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef PARAGRAPHBULLETSNUMBERS_H
#define PARAGRAPHBULLETSNUMBERS_H

#include <ui_ParagraphBulletsNumbers.h>
#include "../ListItemsHelper.h"

#include <KoListStyle.h>

#include <QWidget>

class KoParagraphStyle;

class ParagraphBulletsNumbers : public QWidget
{
    Q_OBJECT
public:
    ParagraphBulletsNumbers(QWidget *parent);

    void setDisplay(KoParagraphStyle *style, int level = 0);

    void save(KoParagraphStyle *style);

    int addStyle(const Lists::ListStyleItem &lsi);

private slots:
    void styleChanged(int);
    void customCharButtonPressed();

private:
    Ui::ParagraphBulletsNumbers widget;

    QHash<int, KoListStyle::Style> m_mapping;
    int m_previousLevel;
    int m_blankCharIndex;
};

#endif
