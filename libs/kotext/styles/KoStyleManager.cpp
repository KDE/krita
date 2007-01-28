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
    static int s_stylesNumber; // For giving out unique numbers to the styles for referencing

    QList<KoCharacterStyle*> m_charStyles;
    QList<KoParagraphStyle*> m_paragStyles;
    QList<ChangeFollower*> m_documentUpdaterProxies;

    bool m_updateTriggered;
    QList<int> m_updateQueue;

    KoParagraphStyle *m_standard;
};

// static
int KoStyleManager::Private::s_stylesNumber = 100;

KoStyleManager::KoStyleManager(QObject *parent)
    : QObject(parent), d(new Private())
{
    d->m_updateTriggered = false;
    d->m_standard = new KoParagraphStyle();
    d->m_standard->setLeftMargin(0);
    d->m_standard->setTopMargin(0);
    d->m_standard->setBottomMargin(0);
    d->m_standard->setRightMargin(0);
    d->m_standard->setTextIndent(0);
    d->m_standard->setIndent(0);
    d->m_standard->setAlignment(Qt::AlignLeft);
    d->m_standard->setName( i18n("[No Paragraph Style]"));
    add(d->m_standard);
/* for testing ;)
KoParagraphStyle *a = new KoParagraphStyle();
a->setName("A");
a->setParent(standard);
a->setLeftMargin(40);
add(a);
KoParagraphStyle *b = new KoParagraphStyle();
b->setName("B");
b->setParent(standard);
b->setRightMargin(400);
add(b); */
}

KoStyleManager::~KoStyleManager() {
    delete d;
}

void KoStyleManager::add(KoCharacterStyle *style) {
    if(d->m_charStyles.contains(style))
        return;
    style->setStyleId( d->s_stylesNumber++ );
    d->m_charStyles.append(style);
}

void KoStyleManager::add(KoParagraphStyle *style) {
    if(d->m_paragStyles.contains(style))
        return;
    style->setStyleId( d->s_stylesNumber++ );
    d->m_paragStyles.append(style);
    if(style->characterStyle())
        add(style->characterStyle());
}

void KoStyleManager::remove(KoCharacterStyle *style) {
    d->m_charStyles.removeAll(style);
}

void KoStyleManager::remove(KoParagraphStyle *style) {
    d->m_paragStyles.removeAll(style);
}

void KoStyleManager::remove(ChangeFollower *cf) {
    d->m_documentUpdaterProxies.removeAll(cf);
}

void KoStyleManager::alteredStyle(const KoParagraphStyle *style) {
    Q_ASSERT(style);
    int id = style->styleId();
    if(id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!" << endl;
        return;
    }
    if(! d->m_updateQueue.contains(id))
        d->m_updateQueue.append(id);
    requestFireUpdate();

    // check if anyone that uses 'style' as a parent needs to be flagged as changed as well.
    foreach(KoParagraphStyle *ps, d->m_paragStyles) {
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
    if(! d->m_updateQueue.contains(id))
        d->m_updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::updateAlteredStyles() {
    foreach(ChangeFollower *cf, d->m_documentUpdaterProxies) {
        cf->processUpdates(d->m_updateQueue);
    }
    d->m_updateQueue.clear();
    d->m_updateTriggered = false;
}

void KoStyleManager::requestFireUpdate() {
    if(d->m_updateTriggered)
        return;
    QTimer::singleShot(0, this, SLOT(updateAlteredStyles()));
    d->m_updateTriggered = true;
}

void KoStyleManager::add(QTextDocument *document) {
    foreach(ChangeFollower *cf, d->m_documentUpdaterProxies) {
        if(cf->document() == document) {
            return; // already present.
        }
    }
    ChangeFollower *cf = new ChangeFollower(document, this);
    d->m_documentUpdaterProxies.append(cf);
}

void KoStyleManager::remove(QTextDocument *document) {
    foreach(ChangeFollower *cf, d->m_documentUpdaterProxies) {
        if(cf->document() == document) {
            d->m_documentUpdaterProxies.removeAll(cf);
            return;
        }
    }
}

KoCharacterStyle *KoStyleManager::characterStyle(int id) const {
    foreach(KoCharacterStyle *style, d->m_charStyles) {
        if(style->styleId() == id)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(int id) const {
    foreach(KoParagraphStyle *style, d->m_paragStyles) {
        if(style->styleId() == id)
            return style;
    }
    return 0;
}

KoCharacterStyle *KoStyleManager::characterStyle(const QString &name) const {
    foreach(KoCharacterStyle *style, d->m_charStyles) {
        if(style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(const QString &name) const {
    foreach(KoParagraphStyle *style, d->m_paragStyles) {
        if(style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::defaultParagraphStyle() const {
    return d->m_standard;
}

#include "KoStyleManager.moc"
