/* This file is part of the KDE project
   Copyright 2007 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KritaShapeTool.h"
#include <QPainter>
#include <QGridLayout>
#include <QToolButton>
#include <QLabel>
#include <QImage>
#include <QAction>

#include <KAction>
#include <klocale.h>
#include <kiconloader.h>
#include <KFileDialog>

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoImageData.h>

#include "KritaShape.h"
#include <kis_debug.h>


#include "KritaShapeTool.moc"

KritaShapeTool::KritaShapeTool(KoCanvasBase* canvas)
        : KoTool(canvas),
        m_kritaShapeshape(0)
{
}

KritaShapeTool::~KritaShapeTool()
{
}

void KritaShapeTool::activate(bool temporary)
{
    Q_UNUSED(temporary);

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach(KoShape* shape, selection->selectedShapes()) {
        m_kritaShapeshape = dynamic_cast<KritaShape*>(shape);
        if (m_kritaShapeshape)
            break;
    }
    if (!m_kritaShapeshape) {
        emit done();
        return;
    }

    // Create an action to set the krita shape to a simple qimage
    // shape.
    // XXX: find better description!
    KAction * action = new KAction(i18n("Convert Color Managed Image to unmanaged Image"), this);
    addAction("convert_to_qimage", action);
    action->setToolTip(i18n("Remove color management from this image and convert to RGB."));
    connect(action, SIGNAL(triggered()), this, SLOT(textDefaultFormat()));

    // setup the context list.
    QList<QAction*> list;
    list.append(this->action("convert_to_qimage"));
    setPopupActionList(list);

    useCursor(Qt::ArrowCursor, true);
}

void KritaShapeTool::deactivate()
{
    m_kritaShapeshape = 0;
}

void KritaShapeTool::paint(QPainter& painter, const KoViewConverter& viewConverter)
{
    Q_UNUSED(painter);
    Q_UNUSED(viewConverter);
}

void KritaShapeTool::mousePressEvent(KoPointerEvent*)
{
}

void KritaShapeTool::mouseMoveEvent(KoPointerEvent*)
{
}

void KritaShapeTool::mouseReleaseEvent(KoPointerEvent*)
{
}


QWidget * KritaShapeTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);

    QToolButton *button = 0;

    QLabel * lbl = new QLabel(i18n("Import image"), optionWidget);
    layout->addWidget(lbl, 0, 0);

    button = new QToolButton(optionWidget);
    button->setIcon(SmallIcon("open"));
    button->setToolTip(i18n("Open"));
    layout->addWidget(button, 0, 1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(slotChangeUrl()));

    return optionWidget;

}

void KritaShapeTool::slotChangeUrl()
{
    KUrl url = KFileDialog::getOpenUrl();
    if (!url.isEmpty() && m_kritaShapeshape)
        m_kritaShapeshape->importImage(url);
}

void KritaShapeTool::slotConvertToQImage()
{
    QImage image = m_kritaShapeshape->convertToQImage();

    // XXX: is it okay to create a new constructor which takes a
    // QImage on KoImageData?
    //KoImageData * imageData = new KoImageData( image );

    // XXX: or should we delete the krita shape, create a picture
    // shape and set the imageData object on that?
    //m_kritaShapeshape->KoShape::setUserData( imageData );

}
