/* This file is part of the KDE project
 * Copyright (C) 2007 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
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

#ifndef TEXTINSERTTEXTCOMMAND_H
#define TEXTINSERTTEXTCOMMAND_H

#include "TextTool.h"
#include "KoTextDocument.h"


#include <QUndoCommand>
#include <QTextCursor>
#include <QPointer>
#include <QString>
#include <QTextCharFormat>
#include <QTextDocument>

/// The undo / redo command for inserting text
class TextInsertTextCommand : public QUndoCommand
{
public:
    /**
     * Command for inserting text.
     *
     * This command insert text on the current QTextCursor
     *
     * @param document the document currently being edited
     * @param tool the text tool active
     * @param caret the cursor editing the document
     * @param text the text to insert
     * @param parent the parent command used for macro commands
     */
    TextInsertTextCommand( TextTool *tool, QString text, QUndoCommand *parent = 0 );

    ~TextInsertTextCommand();

    /// redo the command
	void redo();
    /// revert the actions done in redo
	void undo();
    /// 
	int id() const;

	bool mergeWith(const QUndoCommand *other);

private:
	QString m_text;
	int m_position;
	QTextCharFormat m_charFormat;
	int m_id;

        QPointer<TextTool> m_tool;
};

#endif //TEXTINSERTTEXTCOMMAND_H
