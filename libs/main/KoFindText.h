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

#include <QMetaTypeId>
#include <QTextCursor>

class QTextDocument;
class QTextCharFormat;
class KoCanvasBase;
class KoShape;
/**
 * \brief KoFindBase implementation for searching within text shapes.
 *
 * This class provides a link between KoFindBase and QTextDocument for searching.
 * It uses a list of QTextDocument instances and searches through them using
 * QTextDocument::find().
 *
 * The following options are defined:
 * <ul>
 *      <li><strong>caseSensitive</strong>: Boolean. Default false. Use case-sensitive searching.</li>
 *      <li><strong>wholeWords</strong>: Boolean. Default false. Only match whole words, not parts.</li>
 *      <li><strong>fromCursor</strong>: Boolean. Default true. Start searching from the current cursor
 *          set through setCurrentCursor().</li>
 * </ul>
 *
 * \note Before you can use this class, be sure to set a list of QTextDocuments
 * using setDocuments().
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
     */
    KoFindText(QObject *parent = 0);
    virtual ~KoFindText();

    /**
     * Overridden from KoFindBase
     */
    virtual void findNext();
    /**
     * Overridden from KoFindBase
     */
    virtual void findPrevious();

    /**
     * Retrieve the list of documents currently in use.
     */
    QList<QTextDocument*> documents() const;

    /**
     * Set the current cursor.
     * Used for the "Start from cursor" find option.
     *
     * \param cursor The current cursor.
     */
    virtual void setCurrentCursor(const QTextCursor &cursor);

    /**
     * Set the format to use.
     *
     * Use this function if you want to overwrite the default formatting options.
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
     * Set the list of documents that can be searched.
     *
     * Since there is no way of knowing how an application deals with text shapes
     * and the QTextDocument instances inside those shapes, it is important to
     * update the list of documents whenever something changes.
     *
     * \param documents The list of documents to search through.
     */
    void setDocuments(const QList<QTextDocument*> &documents);

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
