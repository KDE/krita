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

#include "KoStyleManager.h"
#include "KoParagraphStyle.h"
#include "KoCharacterStyle.h"
#include "ChangeFollower.h"

#include <QTimer>
#include <kdebug.h>
#include <klocale.h>

class KoStyleManager::Private
{
public:
    Private() : updateTriggered(false) { }
    static int s_stylesNumber; // For giving out unique numbers to the styles for referencing

    QList<KoCharacterStyle*> charStyles;
    QList<KoParagraphStyle*> paragStyles;
    QList<ChangeFollower*> documentUpdaterProxies;

    bool updateTriggered;
    QList<int> updateQueue;

    KoParagraphStyle *standard;
};

// static
int KoStyleManager::Private::s_stylesNumber = 100;

KoStyleManager::KoStyleManager(QObject *parent)
    : QObject(parent), d(new Private())
{
    d->standard = new KoParagraphStyle();
    d->standard->setLeftMargin(0);
    d->standard->setTopMargin(0);
    d->standard->setBottomMargin(0);
    d->standard->setRightMargin(0);
    d->standard->setTextIndent(0);
    d->standard->setIndent(0);
    d->standard->setAlignment(Qt::AlignLeft);
    d->standard->setName( i18n("[No Paragraph Style]"));
    d->standard->characterStyle()->setName(i18n("[No Character Style]"));
    add(d->standard);
}

KoStyleManager::~KoStyleManager() {
    delete d;
}

void KoStyleManager::add(KoCharacterStyle *style) {
    if(d->charStyles.contains(style))
        return;
    style->setStyleId( d->s_stylesNumber++ );
    d->charStyles.append(style);
}

void KoStyleManager::add(KoParagraphStyle *style) {
    if(d->paragStyles.contains(style))
        return;
    style->setStyleId( d->s_stylesNumber++ );
    d->paragStyles.append(style);
    if(style->characterStyle()) {
        add(style->characterStyle());
        if(style->characterStyle()->name().isEmpty())
            style->characterStyle()->setName(style->name());
    }
}

void KoStyleManager::remove(KoCharacterStyle *style) {
    d->charStyles.removeAll(style);
}

void KoStyleManager::remove(KoParagraphStyle *style) {
    d->paragStyles.removeAll(style);
}

void KoStyleManager::remove(ChangeFollower *cf) {
    d->documentUpdaterProxies.removeAll(cf);
}

void KoStyleManager::alteredStyle(const KoParagraphStyle *style) {
    Q_ASSERT(style);
    int id = style->styleId();
    if(id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!" << endl;
        return;
    }
    if(! d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();

    // check if anyone that uses 'style' as a parent needs to be flagged as changed as well.
    foreach(KoParagraphStyle *ps, d->paragStyles) {
        if(ps->parent() == style)
            alteredStyle(ps);
    }
}

void KoStyleManager::alteredStyle(const KoCharacterStyle *style) {
    Q_ASSERT(style);
    int id = style->styleId();
    if(id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!" << endl;
        return;
    }
    if(! d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::updateAlteredStyles() {
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        cf->processUpdates(d->updateQueue);
    }
    d->updateQueue.clear();
    d->updateTriggered = false;
}

void KoStyleManager::requestFireUpdate() {
    if(d->updateTriggered)
        return;
    QTimer::singleShot(0, this, SLOT(updateAlteredStyles()));
    d->updateTriggered = true;
}

void KoStyleManager::add(QTextDocument *document) {
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        if(cf->document() == document) {
            return; // already present.
        }
    }
    ChangeFollower *cf = new ChangeFollower(document, this);
    d->documentUpdaterProxies.append(cf);
}

void KoStyleManager::remove(QTextDocument *document) {
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        if(cf->document() == document) {
            d->documentUpdaterProxies.removeAll(cf);
            return;
        }
    }
}

KoCharacterStyle *KoStyleManager::characterStyle(int id) const {
    foreach(KoCharacterStyle *style, d->charStyles) {
        if(style->styleId() == id)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(int id) const {
    foreach(KoParagraphStyle *style, d->paragStyles) {
        if(style->styleId() == id)
            return style;
    }
    return 0;
}

KoCharacterStyle *KoStyleManager::characterStyle(const QString &name) const {
    foreach(KoCharacterStyle *style, d->charStyles) {
        if(style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(const QString &name) const {
    foreach(KoParagraphStyle *style, d->paragStyles) {
        if(style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::defaultParagraphStyle() const {
    return d->standard;
}

QList<KoCharacterStyle*> KoStyleManager::characterStyles() const {
    return d->charStyles;
}

QList<KoParagraphStyle*> KoStyleManager::paragraphStyles() const {
    return d->paragStyles;
}

#include "KoStyleManager.moc"
