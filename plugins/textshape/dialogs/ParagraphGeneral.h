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

#ifndef PARAGRAPHGENERAL_H
#define PARAGRAPHGENERAL_H

#include <ui_ParagraphGeneral.h>

#include <QWidget>
#include <QList>

class KoParagraphStyle;
class KoUnit;
class ParagraphBulletsNumbers;
class ParagraphIndentSpacing;
class ParagraphLayout;
class ParagraphDecorations;

class ParagraphGeneral : public QWidget
{
    Q_OBJECT
public:
    ParagraphGeneral(QWidget *parent = 0);

    void setStyle(KoParagraphStyle *style, int level = 0);
    void setParagraphStyles(const QList<KoParagraphStyle*> styles);
    void setUnit(const KoUnit &unit);

    void switchToGeneralTab();
    void hideStyleName(bool hide);

public slots:
    void save(KoParagraphStyle *style = 0);

signals:
    void nameChanged(const QString &name);
    void styleAltered(const KoParagraphStyle *style);

private slots:
    void setName(const QString &name);
    void backgroundColorChanged(QColor);
    void horizontalAlignmentChanged(Qt::Alignment);

private:
    Ui::ParagraphGeneral widget;
    bool m_blockSignals;
    bool m_nameHidden;

    ParagraphIndentSpacing *m_paragraphIndentSpacing;
    ParagraphLayout *m_paragraphLayout;
    ParagraphBulletsNumbers *m_paragraphBulletsNumbers;
    ParagraphDecorations *m_paragraphDecorations;

    KoParagraphStyle *m_style;
    QList<KoParagraphStyle*> m_paragraphStyles;
};

#endif
