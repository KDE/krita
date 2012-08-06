/* This file is part of the KDE project

   Copyright 2007 Montel Laurent <montel@kde.org>
   Copyright 2012 Inge Wallin <inge@lysator.liu.se>

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

// Own
#include "TemplateTool.h"

// Qt
#include <QToolButton>
#include <QGridLayout>

// KDE
#include <KLocale>
#include <KUrl>
#include <KFileDialog>
#include <KIO/Job>

// Calligra
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoIcon.h>

// This shape
#include "TemplateShape.h"
//#include "ChangeSomethingCommand.h"


TemplateTool::TemplateTool( KoCanvasBase* canvas )
    : KoToolBase( canvas ),
      m_templateshape(0)
{
}

void TemplateTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);

    foreach (KoShape *shape, shapes) {
        m_templateshape = dynamic_cast<TemplateShape*>( shape );
        if (m_templateshape)
            break;
    }

    if (!m_templateshape) {
        emit done();
        return;
    }

    useCursor(Qt::ArrowCursor);
}

void TemplateTool::deactivate()
{
    m_templateshape = 0;
}

QWidget * TemplateTool::createOptionWidget()
{
    // Template: This function should not be implemented if the tool
    // does not have an option widget.

    // Template: Example code: a single button that opens a file.
    // Another alternative is to have the option widget in a separate class.

    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);

    QToolButton *button = 0;

    button = new QToolButton(optionWidget);
    button->setIcon(koIcon("document-open"));
    button->setToolTip(i18n( "Open"));
    layout->addWidget(button, 0, 0);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(openPressed()));

    return optionWidget;
}


// ----------------------------------------------------------------
//                         Private slots


void TemplateTool::openPressed()
{
    if (m_templateshape == 0)
        return;

    KUrl url = KFileDialog::getOpenUrl();
    if (!url.isEmpty()) {
        // Template: do something with the file here.
    }
}


// ----------------------------------------------------------------
//                         Event handling


void TemplateTool::mousePressEvent(KoPointerEvent *event)
{
    if (event->button() == Qt::RightButton) {
        event->ignore();
    }
}

void TemplateTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    // Template: Example code: if the shape is double clicked, do the
    //           same as if the button in the option widget is pressed.

    if (canvas()->shapeManager()->shapeAt(event->point) != m_templateshape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    openPressed();
}

#include <TemplateTool.moc>
