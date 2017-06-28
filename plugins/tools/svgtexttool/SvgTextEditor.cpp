/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
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

#include "SvgTextEditor.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QPushButton>
#include <QDebug>

#include <klocalizedstring.h>

#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"

#include <kis_file_name_requester.h>
#include <BasicXMLSyntaxHighlighter.h>

SvgTextEditor::SvgTextEditor(QWidget *parent, Qt::WindowFlags flags)
    : KoDialog(parent, flags)
    , m_page(new QWidget(this))
    , m_shape(0)
{
    widget.setupUi(m_page);
    setMainWidget(m_page);

    BasicXMLSyntaxHighlighter *hl = new BasicXMLSyntaxHighlighter(widget.textEdit);
    Q_UNUSED(hl);

    connect(this, SIGNAL(okClicked()), SLOT(save()));
    connect(widget.bnUndo, SIGNAL(clicked()), widget.textEdit, SLOT(undo()));
    connect(widget.bnRedo, SIGNAL(clicked()), widget.textEdit, SLOT(redo()));
    connect(widget.bnCopy, SIGNAL(clicked()), widget.textEdit, SLOT(copy()));
    connect(widget.bnCut, SIGNAL(clicked()), widget.textEdit, SLOT(cut()));
    connect(widget.bnPaste, SIGNAL(clicked()), widget.textEdit, SLOT(paste()));

    connect(widget.bnBold, SIGNAL(clicked()), this, SLOT(setTextBold()));
    connect(widget.bnItalic, SIGNAL(clicked()), this, SLOT(setTextItalic()));
    connect(widget.bnUnderline, SIGNAL(clicked()), this, SLOT(setTextUnderline()));
    connect(widget.bnStrikethrough, SIGNAL(clicked()), this, SLOT(setTextStrikeThrough()));
    connect(widget.bnTextFgColor, SIGNAL(changed(KoColor)), this, SLOT(setTextFill()));
    connect(widget.bnTextBgColor, SIGNAL(changed(KoColor)), this, SLOT(setTextStroke()));
    connect(widget.bnSuperscript, SIGNAL(clicked()), this, SLOT(setSuperscript()));
    connect(widget.bnSubscript, SIGNAL(clicked()), this, SLOT(setSubscript()));
    connect(widget.fontComboBox, SIGNAL(currentFontChanged(const QFont)), this, SLOT(setFont()));
    connect(widget.fontSize, SIGNAL(editingFinished()), this, SLOT(setSize()));
}

SvgTextEditor::~SvgTextEditor()
{
}

void SvgTextEditor::setShape(KoSvgTextShape *shape)
{
    m_shape = shape;
    if (m_shape) {
        KoSvgTextShapeMarkupConverter converter(m_shape);
        QString svg;
        QString styles;
        if (converter.convertToSvg(&svg, &styles)) {
            //widget.textEdit->setPlainText(QString("%1\n%2").arg(defs).arg(svg));
            widget.textEdit->setPlainText(svg);
        }
        else {
            qWarning() << "Could not get svg text from the shape:" << converter.errors() << converter.warnings();
        }

    }
}

void SvgTextEditor::save()
{
    // We don't do defs or styles yet...
    emit textUpdated(widget.textEdit->document()->toPlainText(), "");
    hide();
}

void SvgTextEditor::setTextBold()
{
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='font-weight:700;'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setTextItalic()
{
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='font-style:italic;'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setTextUnderline()
{
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='text-decoration:underline;'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setTextStrikeThrough()
{
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='text-decoration:line-through;'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setTextFill()
{
    KoColor c = widget.bnTextFgColor->color();
    QColor color = c.toQColor();
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan fill='"+color.name()+"'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setTextStroke()
{
    KoColor c = widget.bnTextBgColor->color();
    QColor color = c.toQColor();
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan stroke='"+color.name()+"'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setFont()
{
    QString fontName = widget.fontComboBox->currentFont().family();
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='font-family:"+fontName+";'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setSize()
{
    QString fontSize = QString::number(widget.fontSize->value());
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='font-size:"+fontSize+";'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setSubscript()
{
    QString fontSize = QString::number(widget.fontSize->value());
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='font-size:50%;baseline-shift:sub;'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 7;
        widget.textEdit->zoomOut(numSteps);
        event->accept();
    }
}

void SvgTextEditor::setSuperscript()
{
    QString fontSize = QString::number(widget.fontSize->value());
    QTextCursor cursor = widget.textEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style='font-size:50%;baseline-shift:super;'>" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}
