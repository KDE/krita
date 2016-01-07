/* This file is part of the KDE project
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "CharacterGeneral.h"

#include <QList>

class KoParagraphStyle;
class KoStyleThumbnailer;
class KoStyleManager;
class KoImageCollection;
class KoUnit;
class ParagraphBulletsNumbers;
class ParagraphIndentSpacing;
class ParagraphLayout;
class ParagraphDecorations;
class ParagraphDropCaps;
class StylesModel;

class ParagraphGeneral : public CharacterGeneral
{
    Q_OBJECT
public:
    explicit ParagraphGeneral(QWidget *parent = 0);

    void setStyle(KoParagraphStyle *style, int level = 0);
    void setUnit(const KoUnit &unit);

    void switchToGeneralTab();
    void hideStyleName(bool hide);
    bool isStyleChanged();
    QString styleName() const;
    void selectName();

    void setImageCollection(KoImageCollection *imageCollection);
    KoImageCollection *imageCollection();
    void setStyleManager(KoStyleManager *sm);

    KoParagraphStyle *style() const;

public Q_SLOTS:
    void save(KoParagraphStyle *style = 0);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void styleAltered(const KoParagraphStyle *style); /// when saving

private Q_SLOTS:
    void setPreviewParagraphStyle();

private:
    bool m_nameHidden;

    ParagraphIndentSpacing *m_paragraphIndentSpacing;
    ParagraphLayout *m_paragraphLayout;
    ParagraphBulletsNumbers *m_paragraphBulletsNumbers;
    ParagraphDecorations *m_paragraphDecorations;
    ParagraphDropCaps *m_paragraphDropCaps;

    KoParagraphStyle *m_style;
    QList<KoParagraphStyle *> m_paragraphStyles;
    KoStyleManager *m_styleManager;

    KoStyleThumbnailer *m_thumbnail;
    StylesModel *m_paragraphInheritedStyleModel;
};

#endif
