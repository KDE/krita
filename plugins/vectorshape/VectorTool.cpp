/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>
   Copyright 2011 Boudewijn Rempt <boud@valdyas.org>

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

#include "VectorTool.h"
#include "VectorShape.h"
#include "ChangeVectorDataCommand.h"

#include <QToolButton>
#include <QGridLayout>
#include <KLocale>
#include <KUrl>
#include <KFileDialog>
#include <KIO/Job>

#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

VectorTool::VectorTool( KoCanvasBase* canvas )
    : KoToolBase( canvas ),
      m_shape(0)
{
}

void VectorTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);

    foreach (KoShape *shape, shapes) {
        m_shape = dynamic_cast<VectorShape*>( shape );
        if ( m_shape )
            break;
    }
    if ( !m_shape )
    {
        emit done();
        return;
    }
    useCursor(Qt::ArrowCursor);
}

void VectorTool::deactivate()
{
  m_shape = 0;
}

QWidget * VectorTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(optionWidget);

    QToolButton *button = 0;

    button = new QToolButton(optionWidget);
    button->setIcon(koIcon("document-open"));
    button->setToolTip(i18n("Open Vector Image (EMF/WMF/SVM)"));
    layout->addWidget(button, 0, 0);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(changeUrlPressed()));

    return optionWidget;
}

void VectorTool::changeUrlPressed()
{
    if (m_shape == 0)
        return;
    const KUrl url = KFileDialog::getOpenUrl(KUrl(), QLatin1String("image/x-emf image/x-wmf"));
    if (!url.isEmpty()) {
        // TODO move this to an action in the libs, with a nice dialog or something.
        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, 0);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(setImageData(KJob*)));
    }

}

void VectorTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    if(canvas()->shapeManager()->shapeAt(event->point) != m_shape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    changeUrlPressed();
}

void VectorTool::setImageData(KJob *job)
{
    if (m_shape == 0) {
        return;
    }
    KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
    Q_ASSERT(transferJob);

    const QByteArray newData = transferJob->data();
    const VectorShape::VectorType vectorType = VectorShape::vectorType(newData);
    ChangeVectorDataCommand *cmd = new ChangeVectorDataCommand(m_shape, qCompress(newData), vectorType);

    canvas()->addCommand(cmd);
}

#include <VectorTool.moc>
