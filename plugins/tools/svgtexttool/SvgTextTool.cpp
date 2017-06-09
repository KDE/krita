/* This file is part of the KDE project

   Copyright 2017 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "SvgTextTool.h"
#include "KoSvgTextShape.h"
#include "SvgTextChangeCommand.h"

#include <QLabel>
#include <QToolButton>
#include <QGridLayout>
#include <QDesktopServices>

#include <klocalizedstring.h>

#include <KisPart.h>

#include <KoFileDialog.h>
#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

#include "SvgTextEditor.h"

SvgTextTool::SvgTextTool(KoCanvasBase *canvas)
    : KoToolBase(canvas)
    , m_shape(0)
    , m_editor(new SvgTextEditor())
{
    connect(m_editor, SIGNAL(textUpdated(QString,QString)), SLOT(textUpdated(QString,QString)));
}

void SvgTextTool::activate(ToolActivation activation, const QSet<KoShape *> &shapes)
{
    KoToolBase::activate(activation, shapes);

    foreach (KoShape *shape, shapes) {
        m_shape = dynamic_cast<KoSvgTextShape *>(shape);
        if (m_shape) {
            break;
        }
    }
    if (!m_shape) {
        emit done();
        return;
    }
    useCursor(Qt::ArrowCursor);
}

void SvgTextTool::deactivate()
{
    m_shape = 0;
    KoToolBase::deactivate();
}

QWidget *SvgTextTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);
    m_edit = new QPushButton(optionWidget);
    m_edit->setText(i18n("Edit Text"));
    connect(m_edit, SIGNAL(clicked(bool)), SLOT(showEditor()));
    layout->addWidget(m_edit);

    return optionWidget;
}

void SvgTextTool::showEditor() const
{
    if (!m_shape) return;
    m_editor->setShape(m_shape);
    m_editor->setModal(false);
    m_editor->show();
}

void SvgTextTool::textUpdated(const QString &svg, const QString &defs)
{
    qDebug() << "text updated" << svg;

    SvgTextChangeCommand *cmd = new SvgTextChangeCommand(m_shape, svg, defs);
    canvas()->addCommand(cmd);
}

void SvgTextTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != m_shape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    showEditor();
}

