/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
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
#ifndef CHANGETRACKER_H
#define CHANGETRACKER_H

#include <QObject>

class QTextDocument;
class TextTool;

class ChangeTracker : public QObject
{
    Q_OBJECT
public:
    explicit ChangeTracker(TextTool *parent);

    void setDocument(QTextDocument *document);

    int getChangeId(QString title, int existingChangeId);

    void notifyForUndo();

private Q_SLOTS:
    void contentsChange(int from, int charsRemoves, int charsAdded);

private:
    QTextDocument *m_document;
    TextTool *m_tool;
    bool m_enableSignals, m_reverseUndo;
    int m_changeId;
};

#endif
