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

#ifndef KOSTYLEMANAGER_H
#define KOSTYLEMANAGER_H

#include <koffice_export.h>

#include <QObject>
#include <QTextDocument>

class KoCharacterStyle;
class KoParagraphStyle;
class ChangeFollower;
class KoStyleManagerPrivate;

class KOTEXT_EXPORT KoStyleManager : public QObject {
    Q_OBJECT
public:
    KoStyleManager(QObject *parent = 0);

    void add(KoCharacterStyle *style);
    void add(KoParagraphStyle *style);
    void remove(KoCharacterStyle *style);
    void remove(KoParagraphStyle *style);

    void add(QTextDocument *document);
    void remove(QTextDocument *document);

    KoCharacterStyle *characterStyle(int id) const;
    KoParagraphStyle *paragraphStyle(int id) const;
    KoCharacterStyle *characterStyle(const QString &name) const;
    KoParagraphStyle *paragraphStyle(const QString &name) const;
    KoParagraphStyle *defaultParagraphStyle() const;

public slots:
    void alteredStyle(const KoParagraphStyle *style);
    void alteredStyle(const KoCharacterStyle *style);

private slots:
    void updateAlteredStyles(); // for the QTimer::singleshot

private:
    friend class ChangeFollower;
    void requestFireUpdate();
    void remove(ChangeFollower *cf);

private:
    static int s_stylesNumber; // For giving out unique numbers to the styles for referencing

    QList<KoCharacterStyle*> m_charStyles;
    QList<KoParagraphStyle*> m_paragStyles;
    QList<ChangeFollower*> m_documentUpdaterProxies;

    bool m_updateTriggered;
    QList<int> m_updateQueue;


    KoParagraphStyle *m_standard;
};

#endif
