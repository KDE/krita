/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

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

#include "PictureTool.h"
#include "PictureShape.h"
#include "ChangeImageCommand.h"

#include <QToolButton>
#include <QGridLayout>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KFileDialog>
#include <KIO/Job>

#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

PictureTool::PictureTool( KoCanvasBase* canvas )
    : KoTool( canvas ),
      m_pictureshape(0)
{
}

void PictureTool::activate (bool temporary)
{
    Q_UNUSED( temporary );

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        m_pictureshape = dynamic_cast<PictureShape*>( shape );
        if ( m_pictureshape )
            break;
    }
    if ( !m_pictureshape )
    {
        emit done();
        return;
    }
    useCursor(Qt::ArrowCursor);
}

void PictureTool::deactivate()
{
  m_pictureshape = 0;
}

QWidget * PictureTool::createOptionWidget()
{

    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);

    QToolButton *button = 0;

    button = new QToolButton(optionWidget);
    button->setIcon(SmallIcon("open"));
    button->setToolTip(i18n( "Open"));
    layout->addWidget(button, 0, 0);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(changeUrlPressed()));

    return optionWidget;
}

void PictureTool::changeUrlPressed()
{
    if (m_pictureshape == 0)
        return;
    KUrl url = KFileDialog::getOpenUrl();
    if (!url.isEmpty()) {
        // TODO move this to an action in the libs, with a nice dialog or something.
        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, 0);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(setImageData(KJob*)));
    }
}

void PictureTool::setImageData(KJob *job)
{
    if (m_pictureshape == 0)
        return; // ugh, the user deselected the image in between. We should move this code to main anyway redesign it
    KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
    Q_ASSERT(transferJob);

    KoImageData *data = m_pictureshape->imageCollection()->createImageData(transferJob->data());
    ChangeImageCommand *cmd = new ChangeImageCommand(m_pictureshape, data);
    m_canvas->addCommand(cmd);
}

void PictureTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    if(m_canvas->shapeManager()->shapeAt(event->point) != m_pictureshape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    changeUrlPressed();
/*
    repaintSelection();
    updateSelectionHandler();
*/
}

#include "PictureTool.moc"
