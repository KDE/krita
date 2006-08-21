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

/**
 * Manages all character and pargraph styles for any number of documents.
 */
class KOTEXT_EXPORT KoStyleManager : public QObject {
    Q_OBJECT
public:
    /**
     * Create a new style manager.
     * @param parent pass a parent to use qobject memory management
     */
    KoStyleManager(QObject *parent = 0);

    /**
     * Add a new style, automatically giving it a new styleId.
     */
    void add(KoCharacterStyle *style);
    /**
     * Add a new style, automatically giving it a new styleId.
     */
    void add(KoParagraphStyle *style);
    /**
     * Remove a style.
     */
    void remove(KoCharacterStyle *style);
    /**
     * Remove a style.
     */
    void remove(KoParagraphStyle *style);

    /**
     * Add a document for which the styles will be applied.
     * Whenever a style is changed (signified by a alteredStyle() call) all
     * registred documents will be updated to reflect that change.
     */
    void add(QTextDocument *document);
     /**
      * Remove a previously registred document.
      */
    void remove(QTextDocument *document);

    /**
     * Return a characterStyle by its id.
     * From documents you can retrieve the id out of each QTextCharFormat
     * by requesting the KoCharacterStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoCharacterStyle::styleId()
     */
    KoCharacterStyle *characterStyle(int id) const;

    /**
     * Return a paragraphStyle by its id.
     * From documents you can retrieve the id out of each QTextBlockFormat
     * by requesting the KoParagraphStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KoParagraphStyle::styleId()
     */
    KoParagraphStyle *paragraphStyle(int id) const;

    /**
     * Return the first characterStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registred with that name, only the first is returned
     * @param the name of the style.
     * @see characterStyle(id);
     */
    KoCharacterStyle *characterStyle(const QString &name) const;

    /**
     * Return the first paragraphStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registred with that name, only the first is returned
     * @param the name of the style.
     * @see paragraphStyle(id);
     */
    KoParagraphStyle *paragraphStyle(const QString &name) const;

    /**
     * Return the default paragraph style that will always be present in each
     * document. You can alter the style, but you can never delete it.
     * The default is suppost to stay invisible to the user and its called
     * i18n("[No Paragraph Style]") for that reason. Applications should not
     * show this style in their document-level configure dialogs.
     */
    KoParagraphStyle *defaultParagraphStyle() const;

public slots:
    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KoParagraphStyle *style);
    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
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
