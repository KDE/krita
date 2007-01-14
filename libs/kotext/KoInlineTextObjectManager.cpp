/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "KoInlineTextObjectManager.h"

#include <QTextCursor>
#include <QPainter>
#include <kdebug.h>

KoInlineTextObjectManager::KoInlineTextObjectManager(QObject *parent)
    : QObject(parent),
    m_lastObjectId(0)
{
}

KoInlineObject *KoInlineTextObjectManager::inlineTextObject(const QTextCharFormat &format) const {
    int id = format.intProperty(InlineInstanceId);
    if(id <= 0)
        return 0;
    return m_objects.value(id);
}

KoInlineObject *KoInlineTextObjectManager::inlineTextObject(const QTextCursor &cursor) const {
    return inlineTextObject(cursor.charFormat());
}

void KoInlineTextObjectManager::insertInlineObject(QTextCursor &cursor, KoInlineObject *object) {
    QTextCharFormat cf;
    cf.setObjectType(1001);
    cf.setProperty(InlineInstanceId, ++m_lastObjectId);
    cursor.insertText(QString(0xFFFC), cf);
    object->setId(m_lastObjectId);
    m_objects.insert(m_lastObjectId, object);
    object->setManager(this);
kDebug() << "calling setup!\n";
    object->setup();
    if(object->propertyChangeListener())
        m_listeners.append(object);
}

/**
 * overloaded method, provided for your convenience.
 */
void KoInlineTextObjectManager::insertInlineObject(QTextCursor &cursor, KoInlineObjectFactory *factory) {
    Q_ASSERT(factory);
    insertInlineObject(cursor, factory->createInlineObject());
}

void KoInlineTextObjectManager::setProperty(KoInlineObject::Property key, QVariant value) {
    if(m_properties.contains(key)) {
        if(value == m_properties.value(key))
            return;
        m_properties.remove(key);
    }
    m_properties.insert(key, value);
    foreach(KoInlineObject *obj, m_listeners)
        obj->propertyChanged(key, value);
}

QVariant KoInlineTextObjectManager::property(KoInlineObject::Property key) const {
    return m_properties.value(key);
}

int KoInlineTextObjectManager::intProperty(KoInlineObject::Property key) const {
    if(!m_properties.contains(key))
        return 0;
    return m_properties.value(key).toInt();
}

bool KoInlineTextObjectManager::boolProperty(KoInlineObject::Property key) const {
    if(!m_properties.contains(key))
        return false;
    return m_properties.value(key).toBool();
}

QString KoInlineTextObjectManager::stringProperty(KoInlineObject::Property key) const {
    if(!m_properties.contains(key))
        return QString();
    return qvariant_cast<QString>(m_properties.value(key));
}

#include "KoInlineTextObjectManager.moc"
