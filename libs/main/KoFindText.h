/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
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



#ifndef KOFINDTEXT_H
#define KOFINDTEXT_H

#include "KoFindBase.h"
#include "komain_export.h"

#include <QtCore/QMetaTypeId>
#include <QTextCursor>

class QTextDocument;
class QTextCharFormat;
class KoCanvasBase;
class KoShape;
/**
 * \brief KoFindBase implementation for searching within text shapes.
 *
 * This class provides a link between KoFindBase and QTextDocument for searching.
 * It uses KoText::CurrentTextDocument for determining what text to search through.
 *
 * Matches created by this implementation use QTextDocument for the container and
 * QTextCursor for the location.
 */
class KOMAIN_EXPORT KoFindText : public KoFindBase
{
    Q_OBJECT
public:
    enum FormatType {
        HighlightFormat,
        CurrentMatchFormat,
        SelectionFormat,
        ReplacedFormat
    };

    /**
     * Constructor.
     *
     * \param provider The current document's resource manager, used for retrieving
     * the actual text to search through.
     */
    KoFindText(const QList<QTextDocument*> &documents, QObject *parent = 0);
    virtual ~KoFindText();

    /**
     * Overridden from KoFindBase
     */
    virtual void findNext();
    /**
     * Overridden from KoFindBase
     */
    virtual void findPrevious();

    virtual void setCurrentCursor(const QTextCursor &cursor);

    /**
     * Set the format use.
     *
     * USe this function if you want to overwrite the default formating options.
     */
    static void setFormat(FormatType formatType, const QTextCharFormat &format);

    /**
     * Helper function to retrieve all QTextDocument objects from a list of shapes.
     *
     * This method will search the list of shapes passed to it recursively for any
     * text shapes. If it encounters any text shapes it will add the QTextDocument
     * object used by that shape to the list passed.
     *
     * \param shapes The shapes to search for text.
     * \param append A list to append the found QTextDocument objects to.
     */
    static void findTextInShapes(const QList<KoShape*> &shapes, QList<QTextDocument*> &append);

public Q_SLOTS:
    /**
     * Append a list of documents to the documents that can be searched.
     *
     * \param documents The list of documents to append.
     */
    void addDocuments(const QList<QTextDocument*> &documents);

protected:
    /**
     * The actual implementation of the searching, overridden from KoFindBase.
     */
    virtual void findImplementation(const QString &pattern, KoFindMatchList &matchList);
    /**
     * The actual implementation of replacing, overridden from KoFindBase.
     */
    virtual void replaceImplementation(const KoFindMatch &match, const QVariant &value);
    /**
     * Clear the list of matches properly. Overridden from KoFindBase.
     */
    virtual void clearMatches();

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void documentDestroyed(QObject* object));
};

Q_DECLARE_METATYPE(QTextDocument *);
Q_DECLARE_METATYPE(QTextCursor);

#endif // KOFINDTEXT_H
