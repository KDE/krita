/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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

#ifndef CHANGEFOLLOWER_H
#define CHANGEFOLLOWER_H

#include "commands/ChangeStylesMacroCommand.h"

#include <KoStyleManager.h>

#include <QObject>
#include <QWeakPointer>
#include <QSet>
#include <QTextDocument>

/**
 * OdfTextTrackStyles is used to update a list of qtextdocument with
 * any changes made in the style manager.
 * 
 * It also creates undo commands and adds them to the undo stack
 *
 * Style changes affect a lot of qtextdocuments and we store the changes and apply
 * the changes to every qtextdocument, so every KoTextDocument has to
 * register their QTextDocument to us.
 *
 * We use the QObject principle of children getting deleted when the
 * parent gets deleted. Thus we die when the KoStyleManager dies.
 */
class OdfTextTrackStyles : public QObject
{

    Q_OBJECT

public:
    static OdfTextTrackStyles *instance(KoStyleManager *manager);

private:
    static QMap<QObject *, OdfTextTrackStyles *> instances;

    OdfTextTrackStyles(KoStyleManager *manager);

    /// Destructor, called when the parent is deleted.
    ~OdfTextTrackStyles();

private Q_SLOTS:
    void beginEdit();
    void endEdit();
    void recordStyleChange(int id, const KoParagraphStyle *origStyle, const KoParagraphStyle *newStyle);
    void recordStyleChange(int id, const KoCharacterStyle *origStyle, const KoCharacterStyle *newStyle);
    void styleManagerDied(QObject *manager);
    void documentDied(QObject *manager);

public:
    void registerDocument(QTextDocument *qDoc);
    void unregisterDocument(QTextDocument *qDoc);

private:
    QList<QTextDocument *> m_documents;
    QWeakPointer<KoStyleManager> m_styleManager;
    ChangeStylesMacroCommand *m_changeCommand;
};

#endif
