/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef TEXTPASTECOMMAND_H
#define TEXTPASTECOMMAND_H

#include <QClipboard>
#include <QWeakPointer>
#include <kundo2command.h>

class QTextDocument;
class KoDocumentRdfBase;
class KoShapeController;
class QMimeData;
class KoCanvasBase;

class TextPasteCommand : public KUndo2Command
{
public:

    TextPasteCommand(const QMimeData *mimeData,
                     QTextDocument *document,
                     KoShapeController *shapeController,
                     KoCanvasBase *canvas, KUndo2Command *parent = 0,
                     bool pasteAsText = false);

    virtual void undo();

    virtual void redo();

private:
    const QMimeData *m_mimeData;
    QWeakPointer<QTextDocument> m_document;
    KoDocumentRdfBase *m_rdf;
    KoShapeController *m_shapeController;
    KoCanvasBase *m_canvas;
    bool m_pasteAsText;
    bool m_first;
};

#endif // TEXTPASTECOMMAND_H
