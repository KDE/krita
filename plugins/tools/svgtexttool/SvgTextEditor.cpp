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
