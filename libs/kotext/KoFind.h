/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#ifndef KOFIND_H
#define KOFIND_H

#include "kotext_export.h"

#include <QObject>

class QTextDocument;
class KoResourceManager;
class KActionCollection;
class KoFindPrivate;

/**
 * This controller class allows you to get the relevant find actions
 * added to your action collection and make them act similarly for all KOffice apps.
 */
class KOTEXT_EXPORT KoFind : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor for the KoFind controller.
     * You can create and forget this class in order to gain find features for your application.
     * @param parent the parent widget, used both as an anchor for the find dialog and for
     *   memory management purposes.
     * @param provider the resource provider for the canvas, used to signal the text shape.
     * @param ac the action collection that the find actions can be added to.
     */
    KoFind(QWidget *parent, KoResourceManager *provider, KActionCollection *ac);
    /// destructor
    ~KoFind();

signals:
    /**
     * @brief This signal is send when the current document has reached its end
     *
     * Connect to this signal if you want to support find in multiple text shapes.
     * In you code you then should select the next text shape and select the text
     * tool.
     *
     * @param document The currently document where find was used.
     */
    void findDocumentSetNext(QTextDocument *document);

    /**
     * @brief This signal is send when the current document has reached its beginning
     *
     * Connect to this signal if you want to support find in multiple text shapes.
     * In you code you then should select the next text shape and select the text
     * tool.
     *
     * @param document The currently document where find was used.
     */
    void findDocumentSetPrevious(QTextDocument *document);

private:
    KoFindPrivate * const d;
    friend class KoFindPrivate;

    Q_PRIVATE_SLOT(d, void resourceChanged(int, const QVariant&))
    Q_PRIVATE_SLOT(d, void findActivated())
    Q_PRIVATE_SLOT(d, void findNextActivated())
    Q_PRIVATE_SLOT(d, void findPreviousActivated())
    Q_PRIVATE_SLOT(d, void replaceActivated())
    Q_PRIVATE_SLOT(d, void startFind())
    Q_PRIVATE_SLOT(d, void startReplace())
};

#endif

