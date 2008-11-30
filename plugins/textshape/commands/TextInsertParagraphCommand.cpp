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

#include "TextInsertParagraphCommand.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextDocumentLayout.h"
#include "KoTextDocument.h"

#include <QTextBlock>
#include <QTextBlockFormat>

#include "kdebug.h"
#include <klocalizedstring.h>

TextInsertParagraphCommand::TextInsertParagraphCommand( TextTool *tool, QUndoCommand *parent )
 : QUndoCommand( i18n("New paragraph"), parent ),
	m_tool(tool)
{
	m_tool->flagUndoRedo( false );
	m_tool->m_caret.beginEditBlock();
	
    QTextBlockFormat format = m_tool->m_caret.blockFormat();

//    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_document->documentLayout());
    KoParagraphStyle *nextStyle = 0;
    if(KoTextDocument(m_tool->m_textShapeData->document()).styleManager()) {
        int id = m_tool->m_caret.blockFormat().intProperty(KoParagraphStyle::StyleId);
        KoParagraphStyle *currentStyle = KoTextDocument(m_tool->m_textShapeData->document()).styleManager()->paragraphStyle(id);
        if(currentStyle == 0) // not a style based parag.  Lets make the next one correct.
            nextStyle = KoTextDocument(m_tool->m_textShapeData->document()).styleManager()->defaultParagraphStyle();
        else
            nextStyle = KoTextDocument(m_tool->m_textShapeData->document()).styleManager()->paragraphStyle(currentStyle->nextStyle());
        Q_ASSERT(nextStyle);
      if (currentStyle == nextStyle)
            nextStyle = 0;
    }
    
    QTextList *list = m_tool->m_caret.block().textList();
kDebug()<<"insert paragraph before insertBlock";
    m_tool->m_caret.insertBlock();
kDebug()<<"insert paragraph after insertBlock";

    QTextBlockFormat bf = m_tool->m_caret.blockFormat();
    bf.setPageBreakPolicy(QTextFormat::PageBreak_Auto);
    bf.clearProperty(KoParagraphStyle::ListStartValue);
    bf.clearProperty(KoParagraphStyle::UnnumberedListItem);
    bf.clearProperty(KoParagraphStyle::IsListHeader);
kDebug()<<"insert paragraph before first setBlockFormat";
    m_tool->m_caret.setBlockFormat(bf);
kDebug()<<"insert paragraph after first setBlockFormat";
    if (nextStyle) {
        QTextBlock block = m_tool->m_caret.block();
        nextStyle->applyStyle(block);
        if (list)
            list->add(block);
    }

            QVariant direction = format.property(KoParagraphStyle::TextProgressionDirection);
            format = m_tool->m_caret.blockFormat();
            if (m_tool->m_textShapeData->pageDirection() != KoText::AutoDirection) { // inherit from shape
                KoText::Direction dir;
                switch (m_tool->m_textShapeData->pageDirection()) {
                case KoText::RightLeftTopBottom:
                    dir = KoText::PerhapsRightLeftTopBottom;
                    break;
                case KoText::LeftRightTopBottom:
                default:
                    dir = KoText::PerhapsLeftRightTopBottom;
                }
                format.setProperty(KoParagraphStyle::TextProgressionDirection, dir);
            } else if (! direction.isNull()) // then we inherit from the previous paragraph.
                format.setProperty(KoParagraphStyle::TextProgressionDirection, direction);
kDebug()<<"insert paragraph before second setBlockFormat";
            m_tool->m_caret.setBlockFormat(format);
kDebug()<<"insert paragraph after second setBlockFormat";
    
    m_tool->m_caret.endEditBlock();
	m_tool->flagUndoRedo( true );

}


TextInsertParagraphCommand::~TextInsertParagraphCommand()
{
}


    /// redo the command
void TextInsertParagraphCommand::redo()
{
	m_tool->flagUndoRedo( false );
	m_document->redo(m_caret);
	m_tool->flagUndoRedo( true );
//	m_tool->updateActions();
}


    /// revert the actions done in redo
void TextInsertParagraphCommand::undo()
{
	m_tool->flagUndoRedo( false );
	m_document->undo(m_caret);
	m_tool->flagUndoRedo( true );
//	m_tool->updateActions();
}
