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

#ifndef PARAGRAPHLAYOUT_H
#define PARAGRAPHLAYOUT_H

#include <ui_ParagraphLayout.h>

#include <QWidget>

class KoParagraphStyle;

class ParagraphLayout : public QWidget
{
    Q_OBJECT
public:
    ParagraphLayout(QWidget *parent);

    void setDisplay(KoParagraphStyle *style);

    void save(KoParagraphStyle *style);

signals:
    void horizontalAlignmentChanged(Qt::Alignment);

private slots:
    void slotAlignChanged();

private:
    Ui::ParagraphLayout widget;
};

#endif
