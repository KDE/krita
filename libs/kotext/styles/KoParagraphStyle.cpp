/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#include "KoParagraphStyle.h"
#include "KoCharacterStyle.h"
#include "KoListStyle.h"
#include "KoTextBlockData.h"

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>

KoParagraphStyle::KoParagraphStyle()
    : m_charStyle(new KoCharacterStyle(this)),
    m_listStyle(0),
    m_parent(0),
    m_next(0)
{
    m_stylesPrivate = new StylePrivate();
    setLineHeightPercent(120);
}

KoParagraphStyle::KoParagraphStyle(const KoParagraphStyle &orig)
    : QObject(0),
    m_charStyle(new KoCharacterStyle(this)),
    m_listStyle(0),
    m_parent(0),
    m_next(0)
{
    m_stylesPrivate = new StylePrivate();
    m_stylesPrivate->copyMissing(orig.m_stylesPrivate);
    m_name = orig.name();
    m_charStyle = orig.m_charStyle;
    m_next = orig.m_next;
    if(orig.m_listStyle) {
        m_listStyle = orig.m_listStyle;
        m_listStyle->addUser();
    }
}

KoParagraphStyle::~KoParagraphStyle() {
    delete m_stylesPrivate;
    m_stylesPrivate = 0;
    m_charStyle = 0; // QObject will delete it.
    if(m_listStyle) {
        m_listStyle->removeUser();
        if(m_listStyle->userCount() == 0)
            delete m_listStyle;
        m_listStyle = 0;
    }
}

void KoParagraphStyle::setParent(KoParagraphStyle *parent) {
    Q_ASSERT(parent != this);
    if(m_parent)
        m_stylesPrivate->copyMissing(m_parent->m_stylesPrivate);
    m_parent = parent;
    if(m_parent)
        m_stylesPrivate->removeDuplicates(m_parent->m_stylesPrivate);
}

void KoParagraphStyle::setProperty(int key, const QVariant &value) {
    if(m_parent) {
        QVariant const *var = m_parent->get(key);
        if(var && (*var) == value) { // same as parent, so its actually a reset.
            m_stylesPrivate->remove(key);
            return;
        }
    }
    m_stylesPrivate->add(key, value);
}

void KoParagraphStyle::remove(int key) {
    m_stylesPrivate->remove(key);
}

QVariant const *KoParagraphStyle::get(int key) const {
    QVariant const *var = m_stylesPrivate->get(key);
    if(var == 0 && m_parent)
        var = m_parent->get(key);
    return var;
}

double KoParagraphStyle::propertyDouble(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0.0;
    return variant->toDouble();
}

int KoParagraphStyle::propertyInt(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return 0;
    return variant->toInt();
}

bool KoParagraphStyle::propertyBoolean(int key) const {
    const QVariant *variant = get(key);
    if(variant == 0)
        return false;
    return variant->toBool();
}

void KoParagraphStyle::applyStyle(QTextBlockFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        QTextFormat::BlockTopMargin,
        QTextFormat::BlockBottomMargin,
        QTextFormat::BlockLeftMargin,
        QTextFormat::BlockRightMargin,
        QTextFormat::BlockAlignment,
        QTextFormat::TextIndent,
        QTextFormat::BlockIndent,
        QTextFormat::BlockNonBreakableLines,
        StyleId,
        FixedLineHeight,
        MinimumLineHeight,
        LineSpacing,
        LineSpacingFromFont,
//       AlignLastLine,
//       WidowThreshold,
//       OrphanThreshold,
//       DropCaps,
//       DropCapsLength,
//       DropCapsLines,
//       DropCapsDistance,
//       FollowDocBaseline,
        BreakBefore,
        BreakAfter,
//       HasLeftBorder,
//       HasTopBorder,
//       HasRightBorder,
//       HasBottomBorder,
//       BorderLineWidth,
//       SecondBorderLineWidth,
//       DistanceToSecondBorder,
//       LeftPadding,
//       TopPadding,
//       RightPadding,
//       BottomPadding,
        -1
    };

    int i=0;
    while(properties[i] != -1) {
        QVariant const *variant = get(properties[i]);
        if(variant)
            format.setProperty(properties[i], *variant);
        else
            format.clearProperty(properties[i]);
        i++;
    }
}

void KoParagraphStyle::applyStyle(QTextBlock &block) const {
    QTextCursor cursor(block);
    QTextBlockFormat format = cursor.blockFormat();
    applyStyle(format);
    cursor.setBlockFormat(format);
    if(m_charStyle)
        m_charStyle->applyStyle(block);

    if(m_listStyle) {
        // make sure this block becomes a list if its not one already
        m_listStyle->applyStyle(block);
    } else if(block.textList()) {
        // remove
        block.textList()->remove(block);
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
        if(data)
            data->setCounterWidth(-1);
    }
}

void KoParagraphStyle::setListStyle(const KoListStyle &style) {
    if(m_listStyle)
        m_listStyle->apply(style);
    else {
        m_listStyle = new KoListStyle(style);
        m_listStyle->addUser();
    }
}

void KoParagraphStyle::removeListStyle() {
    delete m_listStyle; m_listStyle = 0;
}

#include "KoParagraphStyle.moc"
