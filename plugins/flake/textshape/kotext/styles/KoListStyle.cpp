/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2010 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2011-2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "KoListStyle.h"

#include "KoListLevelProperties.h"
#include "KoList.h"

#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include "TextDebug.h"
#include <QBuffer>

class Q_DECL_HIDDEN KoListStyle::Private
{
public:
    Private() : styleId(0) { }

    QString name;
    int styleId;
    QMap<int, KoListLevelProperties> levels;
};

KoListStyle::KoListStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KoListStyle::~KoListStyle()
{
    delete d;
}

bool KoListStyle::operator==(const KoListStyle &other) const
{
    Q_FOREACH (int level, d->levels.keys()) {
        if (! other.hasLevelProperties(level))
            return false;
        if (!(other.levelProperties(level) == d->levels[level]))
            return false;
    }
    Q_FOREACH (int level, other.d->levels.keys()) {
        if (! hasLevelProperties(level))
            return false;
    }
    return true;
}

bool KoListStyle::operator!=(const KoListStyle &other) const
{
    return !KoListStyle::operator==(other);
}

void KoListStyle::copyProperties(KoListStyle *other)
{
    d->styleId = other->d->styleId;
    d->levels = other->d->levels;
    setName(other->name());
}

KoListStyle *KoListStyle::clone(QObject *parent)
{
    KoListStyle *newStyle = new KoListStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

QString KoListStyle::name() const
{
    return d->name;
}

void KoListStyle::setName(const QString &name)
{
    if (d->name == name)
        return;
    d->name = name;
    emit nameChanged(d->name);
}

int KoListStyle::styleId() const
{
    return d->styleId;
}

void KoListStyle::setStyleId(int id)
{
    d->styleId = id;
    Q_FOREACH (int level, d->levels.keys()) {
        d->levels[level].setStyleId(id);
    }
}

KoListLevelProperties KoListStyle::levelProperties(int level) const
{
    if (d->levels.contains(level))
        return d->levels.value(level);

    level = qMax(1, level);
    if (d->levels.count()) {
        KoListLevelProperties llp = d->levels.begin().value();
        llp.setLevel(level);
        return llp;
    }
    KoListLevelProperties llp;
    llp.setLevel(level);
    if (d->styleId)
        llp.setStyleId(d->styleId);
    return llp;
}

QTextListFormat KoListStyle::listFormat(int level) const
{
    KoListLevelProperties llp = levelProperties(level);
    QTextListFormat format;
    llp.applyStyle(format);
    return format;
}

void KoListStyle::setLevelProperties(const KoListLevelProperties &properties)
{
    int level = qMax(1, properties.level());
    refreshLevelProperties(properties);
    emit styleChanged(level);
}

void KoListStyle::refreshLevelProperties(const KoListLevelProperties &properties)
{
    int level = qMax(1, properties.level());
    KoListLevelProperties llp = properties;
    if (isOulineStyle()) {
        llp.setOutlineList(true);
    }
    llp.setLevel(level);
    d->levels.insert(level, llp);
}

bool KoListStyle::hasLevelProperties(int level) const
{
    return d->levels.contains(level);
}

void KoListStyle::removeLevelProperties(int level)
{
    d->levels.remove(level);
}

void KoListStyle::applyStyle(const QTextBlock &block, int level)
{
    KoList::applyStyle(block, this, level);
}

void KoListStyle::loadOdf(KoShapeLoadingContext& scontext, const KoXmlElement& style)
{
    d->name = style.attributeNS(KoXmlNS::style, "display-name", QString());
    // if no style:display-name is given us the style:name
    if (d->name.isEmpty()) {
        d->name = style.attributeNS(KoXmlNS::style, "name", QString());
    }
    d->name = style.attributeNS(KoXmlNS::style, "name", QString());

    KoXmlElement styleElem;
    forEachElement(styleElem, style) {
        KoListLevelProperties properties;
        properties.loadOdf(scontext, styleElem);
        if (d->styleId)
            properties.setStyleId(d->styleId);
        setLevelProperties(properties);
    }

    if (d->levels.isEmpty()) {
        KoListLevelProperties llp;
        llp.setLevel(1);
        llp.setStartValue(1);
        llp.setStyle(KoListStyle::DecimalItem);
        llp.setListItemSuffix(".");
        setLevelProperties(llp);
    }
}

void KoListStyle::saveOdf(KoGenStyle &style, KoShapeSavingContext &context) const
{
    // style:display-name can be used in list styles but not in outline styles
    if (!d->name.isEmpty() && !style.isDefaultStyle() && !isOulineStyle()) {
        style.addAttribute("style:display-name", d->name);
    }
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QMapIterator<int, KoListLevelProperties> it(d->levels);
    while (it.hasNext()) {
        it.next();
        it.value().saveOdf(&elementWriter, context);
    }
    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    style.addChildElement("text-list-level-style-content", elementContents);
}

bool KoListStyle::isNumberingStyle() const
{
    QMap<int, KoListLevelProperties>::const_iterator it(d->levels.constBegin());
    for (; it != d->levels.constEnd(); ++it) {
        if (isNumberingStyle(it.value().style())) {
            return true;
        }
    }
    return false;
}

bool KoListStyle::isNumberingStyle(int style)
{
    bool retval = true;
    switch (style) {
    case KoListStyle::SquareItem:
    case KoListStyle::DiscItem:
    case KoListStyle::CircleItem:
    case KoListStyle::None:
    case KoListStyle::Bullet:
    case KoListStyle::BlackCircle:
    case KoListStyle::BoxItem:
    case KoListStyle::RhombusItem:
    case KoListStyle::HeavyCheckMarkItem:
    case KoListStyle::BallotXItem:
    case KoListStyle::RightArrowItem:
    case KoListStyle::RightArrowHeadItem:
    case KoListStyle::CustomCharItem:
    case KoListStyle::ImageItem:
        retval = false;
        break;
    default:
        retval = true;
    }
    return retval;
}

bool KoListStyle::isOulineStyle() const
{
    QMap<int, KoListLevelProperties>::const_iterator it(d->levels.constBegin());
    for (; it != d->levels.constEnd(); ++it) {
        if (it.value().isOutlineList()) {
            return true;
        }
    }
    return false;
}

QList<int> KoListStyle::listLevels() const
{
    return d->levels.keys();
}

int KoListStyle::bulletCharacter(int style)
{
    int bullet;
    switch (style) {
    case KoListStyle::Bullet:               bullet = 0x2022; break;
    case KoListStyle::CircleItem:           bullet = 0x25CB; break;
    case KoListStyle::RhombusItem:          bullet = 0x25C6; break;
    case KoListStyle::SquareItem:           bullet = 0x25A0; break;
    case KoListStyle::RightArrowHeadItem:   bullet = 0x27A2; break;
    case KoListStyle::RightArrowItem:       bullet = 0x2794; break;
    case KoListStyle::HeavyCheckMarkItem:   bullet = 0x2714; break;
    case KoListStyle::BallotXItem:          bullet = 0x2717; break;
    case KoListStyle::BlackCircle:
    case KoListStyle::DiscItem:             bullet = 0x25CF; break;
    default:                                bullet = 0; break; //empty character
    }
    return bullet;
}
