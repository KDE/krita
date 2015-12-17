/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kde.org>
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
 * Boston, MA 02110-1301, USA.*/

#include "TextPasteCommand.h"

#include <KoText.h>
#include <KoTextEditor.h>
#include <KoTextDocument.h>
#include <KoTextPaste.h>
#include <KoShapeController.h>
#include <KoParagraphStyle.h>

#include <klocalizedstring.h>
#include "TextDebug.h"

#include <QTextDocument>
#include <QMimeData>

#include "DeleteCommand.h"
#include "KoDocumentRdfBase.h"

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#else
namespace Soprano
{
    class Model
    {
    };
}
#endif

TextPasteCommand::TextPasteCommand(const QMimeData *mimeData,
                                   QTextDocument *document,
                                   KoShapeController *shapeController,
                                   KoCanvasBase *canvas, KUndo2Command *parent, bool pasteAsText)
    : KUndo2Command (parent),
      m_mimeData(mimeData),
      m_document(document),
      m_rdf(0),
      m_shapeController(shapeController),
      m_canvas(canvas),
      m_pasteAsText(pasteAsText),
      m_first(true)
{
    m_rdf = qobject_cast<KoDocumentRdfBase*>(shapeController->resourceManager()->resource(KoText::DocumentRdf).value<QObject*>());

    if (m_pasteAsText)
        setText(kundo2_i18n("Paste As Text"));
    else
        setText(kundo2_i18n("Paste"));
}

void TextPasteCommand::undo()
{
    KUndo2Command::undo();
}

void TextPasteCommand::redo()
{
    if (m_document.isNull()) return;

    KoTextDocument textDocument(m_document);
    KoTextEditor *editor = textDocument.textEditor();

    if (!m_first) {
        KUndo2Command::redo();
    } else {
        editor->beginEditBlock(); //this is needed so Qt does not merge successive paste actions together
        m_first = false;
        if (editor->hasSelection()) { //TODO
            editor->addCommand(new DeleteCommand(DeleteCommand::NextChar, m_document.data(), m_shapeController, this));
        }

        // check for mime type
        if (m_mimeData->hasFormat(KoOdf::mimeType(KoOdf::Text))
                        || m_mimeData->hasFormat(KoOdf::mimeType(KoOdf::OpenOfficeClipboard)) ) {
            KoOdf::DocumentType odfType = KoOdf::Text;
            if (!m_mimeData->hasFormat(KoOdf::mimeType(odfType))) {
                odfType = KoOdf::OpenOfficeClipboard;
            }

            if (editor->blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
                editor->insertText("");
            }

            if (m_pasteAsText) {
                editor->insertText(m_mimeData->text());
            } else {

                QSharedPointer<Soprano::Model> rdfModel;
#ifdef SHOULD_BUILD_RDF
                if(!m_rdf) {
                    rdfModel = QSharedPointer<Soprano::Model>(Soprano::createModel());
                } else {
                    rdfModel = m_rdf->model();
                }
#endif

                KoTextPaste paste(editor, m_shapeController, rdfModel, m_canvas, this);
                paste.paste(odfType, m_mimeData);

#ifdef SHOULD_BUILD_RDF
                if (m_rdf) {
                    m_rdf->updateInlineRdfStatements(editor->document());
                }
#endif
            }
        } else if (!m_pasteAsText && m_mimeData->hasHtml()) {
            editor->insertHtml(m_mimeData->html());
        } else if (m_pasteAsText || m_mimeData->hasText()) {
            editor->insertText(m_mimeData->text());
        }
        editor->endEditBlock(); //see above beginEditBlock
    }
}
