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
#include <QAction>
#include <QMenu>
#include <QTabWidget>

#include <klocalizedstring.h>

#include "KoSvgTextShape.h"
#include "KoSvgTextShapeMarkupConverter.h"
#include <KoColorSpaceRegistry.h>

#include <kis_icon.h>
#include <kis_file_name_requester.h>
#include <BasicXMLSyntaxHighlighter.h>

SvgTextEditor::SvgTextEditor(QWidget *parent, Qt::WindowFlags flags)
    : KoDialog(parent, flags)
    , m_page(new QWidget(this))
    , m_shape(0)
{
    widget.setupUi(m_page);
    setMainWidget(m_page);

    BasicXMLSyntaxHighlighter *hl = new BasicXMLSyntaxHighlighter(widget.svgTextEdit);
    Q_UNUSED(hl);

    connect(this, SIGNAL(okClicked()), SLOT(save()));
    connect(widget.bnUndo, SIGNAL(clicked()), widget.svgTextEdit, SLOT(undo()));
    connect(widget.bnRedo, SIGNAL(clicked()), widget.svgTextEdit, SLOT(redo()));
    connect(widget.bnCopy, SIGNAL(clicked()), widget.svgTextEdit, SLOT(copy()));
    connect(widget.bnCut, SIGNAL(clicked()), widget.svgTextEdit, SLOT(cut()));
    connect(widget.bnPaste, SIGNAL(clicked()), widget.svgTextEdit, SLOT(paste()));
    connect(widget.textTab, SIGNAL(currentChanged(int)), this, SLOT(switchTextEditorTab()));
    switchTextEditorTab();

    //make actions here//
    QAction *makeBold = new QAction(KisIconUtils::loadIcon("format-text-bold"), "Bold");
    connect(makeBold, SIGNAL(triggered(bool)), this, SLOT(setTextBold()));
    QAction *makeWeightLight = new QAction("Light");
    connect(makeWeightLight, SIGNAL(triggered(bool)), this, SLOT(setTextWeightLight()));
    QAction *makeWeightNormal = new QAction("Normal");
    connect(makeWeightNormal, SIGNAL(triggered(bool)), this, SLOT(setTextWeightNormal()));
    QAction *makeWeightDemi = new QAction("Demi-Bold");
    connect(makeWeightDemi, SIGNAL(triggered(bool)), this, SLOT(setTextWeightDemi()));
    QAction *makeWeightBlack = new QAction("Black");
    connect(makeWeightBlack, SIGNAL(triggered(bool)), this, SLOT(setTextWeightBlack()));
    widget.bnBold->setDefaultAction(makeBold);
    QList<QAction*> textWeightActions;
    textWeightActions.append(makeWeightLight);
    textWeightActions.append(makeWeightNormal);
    textWeightActions.append(makeWeightDemi);
    textWeightActions.append(makeBold);
    textWeightActions.append(makeWeightBlack);
    QMenu *textWeight = new QMenu();
    textWeight->addActions(textWeightActions);
    widget.bnBold->setMenu(textWeight);

    QAction *makeItalic = new QAction(KisIconUtils::loadIcon("format-text-italic"), "Italic");
    connect(makeItalic, SIGNAL(triggered(bool)), this, SLOT(setTextItalic()));
    widget.bnItalic->setDefaultAction(makeItalic);

    QAction *makeUnderline = new QAction(KisIconUtils::loadIcon("format-text-underline"), "Underline");
    connect(makeUnderline, SIGNAL(triggered(bool)), this, SLOT(setTextUnderline()));
    widget.bnUnderline->setDefaultAction(makeUnderline);
    QAction *makeStrike = new QAction(KisIconUtils::loadIcon("format-text-strikethrough"), "Strikethrough");
    connect(makeStrike, SIGNAL(triggered(bool)), this, SLOT(setTextStrikethrough()));
    QAction *makeOverline = new QAction("Overline");
    connect(makeOverline, SIGNAL(triggered(bool)), this, SLOT(setTextOverline()));
    QList<QAction*> textDecorActions;
    textDecorActions.append(makeUnderline);
    textDecorActions.append(makeStrike);
    textDecorActions.append(makeOverline);
    QMenu *textDecoration = new QMenu();
    textDecoration->addActions(textDecorActions);
    widget.bnUnderline->setMenu(textDecoration);
    widget.bnStrikethrough->setDefaultAction(makeStrike);

    connect(widget.bnTextFgColor, SIGNAL(changed(KoColor)), this, SLOT(setTextFill()));
    connect(widget.bnTextBgColor, SIGNAL(changed(KoColor)), this, SLOT(setTextStroke()));
    //connect(widget.bnSuperscript, SIGNAL(clicked()), this, SLOT(setSuperscript()));
    //connect(widget.bnSubscript, SIGNAL(clicked()), this, SLOT(setSubscript()));
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
            //widget.svgTextEdit->setPlainText(QString("%1\n%2").arg(defs).arg(svg));
            widget.svgTextEdit->setPlainText(svg);
        }
        else {
            qWarning() << "Could not get svg text from the shape:" << converter.errors() << converter.warnings();
        }

    }
}

void SvgTextEditor::save()
{
    // We don't do defs or styles yet...
    emit textUpdated(widget.svgTextEdit->document()->toPlainText(), "");
    hide();
}

void SvgTextEditor::switchTextEditorTab()
{
    if (widget.textTab->currentIndex() == Richtext) {
        //first, make buttons checkable
        widget.bnBold->setCheckable(true);
        widget.bnItalic->setCheckable(true);
        widget.bnUnderline->setCheckable(true);
        //then connec the cursor change to the checkformat();
        connect(widget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));

        //implement svg to richtext here.
    } else {
        //first, make buttons uncheckable
        widget.bnBold->setCheckable(false);
        widget.bnItalic->setCheckable(false);
        widget.bnUnderline->setCheckable(false);
        disconnect(widget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));

        //implement richtext to svg here.
    }
}

void SvgTextEditor::checkFormat()
{
    QTextCharFormat format = widget.richTextEdit->textCursor().charFormat();
    if (format.fontWeight()>QFont::Normal) {
        widget.bnBold->setChecked(true);
    } else {
        widget.bnBold->setChecked(false);
    }
    widget.bnItalic->setChecked(format.fontItalic());
    widget.bnUnderline->setChecked(format.fontUnderline());
    //widget.fontComboBox->setCurrentFont(format.font());
    widget.fontSize->setValue(format.fontPointSize());
    KoColor color(format.foreground().color(), KoColorSpaceRegistry::instance()->rgb8());
    widget.bnTextFgColor->setColor(color);
}

void SvgTextEditor::setTextBold(QFont::Weight weight)
{
    if (widget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        if (widget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal && weight==QFont::Bold) {
            format.setFontWeight(QFont::Normal);
        } else {
            format.setFontWeight(weight);
        }
        widget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = widget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-weight:700;\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setTextWeightLight()
{
    if (widget.richTextEdit->textCursor().charFormat().fontWeight()<QFont::Normal) {
        setTextBold(QFont::Normal);
    } else {
        setTextBold(QFont::Light);
    }
}

void SvgTextEditor::setTextWeightNormal()
{
    setTextBold(QFont::Normal);
}

void SvgTextEditor::setTextWeightDemi()
{
    if (widget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal
            && widget.richTextEdit->textCursor().charFormat().fontWeight()<QFont::Normal) {
        setTextBold(QFont::Normal);
    } else {
    setTextBold(QFont::DemiBold);
    }
}

void SvgTextEditor::setTextWeightBlack()
{
    if (widget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal) {
        setTextBold(QFont::Normal);
    } else {
    setTextBold(QFont::Black);
    }
}

void SvgTextEditor::setTextItalic(QFont::Style style)
{
    QTextCursor cursor = widget.svgTextEdit->textCursor();
    QString fontStyle = "inherit";
    if (style == QFont::StyleItalic) {
        fontStyle = "italic";
    } else if(style == QFont::StyleOblique) {
        fontStyle = "oblique";
    }
    if (widget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontItalic(!widget.richTextEdit->textCursor().charFormat().fontItalic());
        widget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-style:"+fontStyle+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setTextDecoration(KoSvgText::TextDecoration decor)
{
    QTextCursor cursor = widget.svgTextEdit->textCursor();
    QTextCharFormat currentFormat = widget.richTextEdit->textCursor().charFormat();
    QTextCharFormat format;
    QString textDecoration = "inherit";
    if (decor == KoSvgText::DecorationUnderline) {
        textDecoration = "underline";
        if (currentFormat.fontUnderline()) {
        format.setFontUnderline(false);
        } else {
            format.setFontUnderline(true);
        }
        format.setFontOverline(false);
        format.setFontStrikeOut(false);
    } else if (decor == KoSvgText::DecorationLineThrough) {
        textDecoration = "line-through";
        format.setFontUnderline(false);
        format.setFontOverline(false);
        if (currentFormat.fontStrikeOut()) {
        format.setFontStrikeOut(false);
        } else {
            format.setFontStrikeOut(true);
        }
    } else if (decor == KoSvgText::DecorationOverline) {
        textDecoration = "overline";
        format.setFontUnderline(false);
        if (currentFormat.fontOverline()) {
        format.setFontOverline(false);
        } else {
            format.setFontOverline(true);
        }
        format.setFontStrikeOut(false);
    }
    if (widget.textTab->currentIndex() == Richtext) {
        widget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"text-decoration:"+textDecoration+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setTextUnderline()
{
    setTextDecoration();
}

void SvgTextEditor::setTextOverline()
{
    setTextDecoration(KoSvgText::DecorationOverline);
}

void SvgTextEditor::setTextStrikethrough()
{
    setTextDecoration(KoSvgText::DecorationLineThrough);
}

void SvgTextEditor::setTextFill()
{
    KoColor c = widget.bnTextFgColor->color();
    QColor color = c.toQColor();
    QTextEdit t;
    if (widget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setForeground(QBrush(color));
        widget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
    QTextCursor cursor = widget.svgTextEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan fill=\""+color.name()+"\">" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
    }
}

void SvgTextEditor::setTextStroke()
{
    KoColor c = widget.bnTextBgColor->color();
    QColor color = c.toQColor();
    QTextCursor cursor = widget.svgTextEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan stroke=\""+color.name()+"\">" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setFont()
{
    QString fontName = widget.fontComboBox->currentFont().family();
    if (widget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontFamily(fontName);
        widget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = widget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-family:"+fontName+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setSize()
{
    QString fontSize = QString::number(widget.fontSize->value());
    if (widget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontPointSize((qreal)widget.fontSize->value());
        widget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = widget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-size:"+fontSize+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setBaseline(KoSvgText::BaselineShiftMode)
{

    QTextCursor cursor = widget.svgTextEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style=\"font-size:50%;baseline-shift:super;\">" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 7;
        widget.svgTextEdit->zoomOut(numSteps);
        event->accept();
    }
}
