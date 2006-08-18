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

KoStyleManager::KoStyleManager(QObject *parent)
    : QObject(parent),
    m_updateTriggered(false)
{
    m_standard = new KoParagraphStyle();
    m_standard->setLeftMargin(0);
    m_standard->setTopMargin(0);
    m_standard->setBottomMargin(0);
    m_standard->setRightMargin(0);
    m_standard->setTextIndent(0);
    m_standard->setIndent(0);
    m_standard->setAlignment(Qt::AlignLeft);
    m_standard->setName( i18n("[No Paragraph Style]"));
    add(m_standard);
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

void KoStyleManager::add(KoCharacterStyle *style) {
    if(m_charStyles.contains(style))
        return;
    style->setStyleId( s_stylesNumber++ );
    m_charStyles.append(style);
}

void KoStyleManager::add(KoParagraphStyle *style) {
    if(m_paragStyles.contains(style))
        return;
    style->setStyleId( s_stylesNumber++ );
    m_paragStyles.append(style);
}

void KoStyleManager::remove(KoCharacterStyle *style) {
    m_charStyles.removeAll(style);
}

void KoStyleManager::remove(KoParagraphStyle *style) {
    m_paragStyles.removeAll(style);
}

void KoStyleManager::remove(ChangeFollower *cf) {
    m_documentUpdaterProxies.removeAll(cf);
}

void KoStyleManager::alteredStyle(const KoParagraphStyle *style) {
    Q_ASSERT(style);
    int id = style->styleId();
    if(id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registred style!" << endl;
        return;
    }
    if(! m_updateQueue.contains(id))
        m_updateQueue.append(id);
    requestFireUpdate();

    // check if anyone that uses 'style' as a parent needs to be flagged as changed as well.
    foreach(KoParagraphStyle *ps, m_paragStyles) {
        if(ps->parent() == style)
            alteredStyle(ps);
    }
}

void KoStyleManager::alteredStyle(const KoCharacterStyle *style) {
    Q_ASSERT(style);
    int id = style->styleId();
    if(id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registred style!" << endl;
        return;
    }
    if(! m_updateQueue.contains(id))
        m_updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::updateAlteredStyles() {
    foreach(ChangeFollower *cf, m_documentUpdaterProxies) {
        cf->processUpdates(m_updateQueue);
    }
    m_updateQueue.clear();
    m_updateTriggered = false;
}

void KoStyleManager::requestFireUpdate() {
    if(m_updateTriggered)
        return;
    QTimer::singleShot(0, this, SLOT(updateAlteredStyles()));
    m_updateTriggered = true;
}

void KoStyleManager::add(QTextDocument *document) {
    foreach(ChangeFollower *cf, m_documentUpdaterProxies) {
        if(cf->document() == document) {
            return; // already present.
        }
    }
    ChangeFollower *cf = new ChangeFollower(document, this);
    m_documentUpdaterProxies.append(cf);
}

void KoStyleManager::remove(QTextDocument *document) {
    foreach(ChangeFollower *cf, m_documentUpdaterProxies) {
        if(cf->document() == document) {
            m_documentUpdaterProxies.removeAll(cf);
            return;
        }
    }
}

KoCharacterStyle *KoStyleManager::characterStyle(int id) const {
    foreach(KoCharacterStyle *style, m_charStyles) {
        if(style->styleId() == id)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(int id) const {
    foreach(KoParagraphStyle *style, m_paragStyles) {
        if(style->styleId() == id)
            return style;
    }
    return 0;
}

KoCharacterStyle *KoStyleManager::characterStyle(const QString &name) const {
    foreach(KoCharacterStyle *style, m_charStyles) {
        if(style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(const QString &name) const {
    foreach(KoParagraphStyle *style, m_paragStyles) {
        if(style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::defaultParagraphStyle() const {
    return m_standard;
}

// static
int KoStyleManager::s_stylesNumber = 100;

#include "KoStyleManager.moc"
