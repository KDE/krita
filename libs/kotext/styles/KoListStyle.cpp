/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2010 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#include "KoTextBlockData.h"
#include "KoParagraphStyle.h"
#include "KoList.h"

#include <KoStyleStack.h>
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <kdebug.h>
#include <QTextCursor>
#include <QBuffer>

class KoListStyle::Private
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
    foreach(int level, d->levels.keys()) {
        if (! other.hasLevelProperties(level))
            return false;
        if (!(other.levelProperties(level) == d->levels[level]))
            return false;
    }
    foreach(int level, other.d->levels.keys()) {
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
    foreach(int level, d->levels.keys()) {
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

void KoListStyle::saveOdf(KoGenStyle &style)
{
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QMapIterator<int, KoListLevelProperties> it(d->levels);
    while (it.hasNext()) {
        it.next();
        it.value().saveOdf(&elementWriter);
    }
    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    style.addChildElement("text-list-level-style-content", elementContents);
}

bool KoListStyle::isNumberingStyle(int style)
{
    return !(style == KoListStyle::SquareItem || style == KoListStyle::DiscItem 
             || style == KoListStyle::CircleItem || style == KoListStyle::BoxItem 
             || style == KoListStyle::RhombusItem || style == KoListStyle::HeavyCheckMarkItem 
             || style == KoListStyle::BallotXItem || style == KoListStyle::RightArrowItem 
             || style == KoListStyle::RightArrowHeadItem);
}

QList<int> KoListStyle::listLevels() const
{
    return d->levels.keys();
}
