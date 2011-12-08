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
#include "CropWidget.h"

#include <QToolButton>
#include <QComboBox>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KFileDialog>
#include <KIO/Job>

#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoFilterEffect.h>
#include <KoFilterEffectRegistry.h>
#include <KoFilterEffectConfigWidgetBase.h>
#include <KoFilterEffectStack.h>

PictureTool::PictureTool( KoCanvasBase* canvas )
    : KoToolBase( canvas ),
      m_pictureshape(0),
      m_filterEffect(0),
      m_cropWidget(0)
{
}

void PictureTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);

    foreach (KoShape *shape, shapes) {
        if ((m_pictureshape = dynamic_cast<PictureShape*>(shape)))
            break;
    }
    
    if (!m_pictureshape) {
        emit done();
        return;
    }

    if(m_cropWidget) {
        m_cropWidget->setPictureShape(m_pictureshape);
        m_cropWidget->update();
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
    QVBoxLayout *layout = new QVBoxLayout(optionWidget);
    QHBoxLayout *hBox   = new QHBoxLayout();
    
//     QGridLayout *layout = new QGridLayout(optionWidget);

    QToolButton *button = new QToolButton;
    button->setIcon(SmallIcon("open"));
    button->setToolTip(i18n("Open"));
    
    m_scrollArea = new QScrollArea();
    
//     layout->addWidget(button, 0, 0);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(changeUrlPressed()));
    
    m_filterEffectsCmb = new QComboBox;
    m_filterEffectsCmb->addItem(i18n("Normal"), "default");
    
    foreach (KoFilterEffectFactoryBase *factory, KoFilterEffectRegistry::instance()->values()) {
        m_filterEffectsCmb->addItem(factory->name(), factory->id());
    }
    
    connect(m_filterEffectsCmb, SIGNAL(currentIndexChanged(int)), this, SLOT(filterSelected(int)));

    m_cropWidget = new CropWidget;
    m_cropWidget->setPictureShape(m_pictureshape);
    
    hBox->addWidget(button);
    hBox->addWidget(m_filterEffectsCmb);
    layout->addLayout(hBox        , 0);
    layout->addWidget(m_scrollArea, 1);
    layout->addWidget(m_cropWidget, 1);
    
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

void PictureTool::filterChanged()
{
//     m_pictureshape->filterEffectStack()->takeFilterEffect(0);
//     m_pictureshape->filterEffectStack()->setClipRect(QRect(0, 0, 100, 100));
    m_pictureshape->update();
}

void PictureTool::filterSelected(int cmbIndex)
{
    QString id = m_filterEffectsCmb->itemData(cmbIndex).toString();
    
    if (id != "default") {
        KoFilterEffectFactoryBase      *filterFactory   = KoFilterEffectRegistry::instance()->get(id);
        KoFilterEffectConfigWidgetBase *filterConfigWdg = filterFactory->createConfigWidget();
        
        m_filterEffect = m_pictureshape->filterEffectStack()->takeFilterEffect(0);
        delete m_filterEffect;
        m_filterEffect = filterFactory->createFilterEffect();
        m_pictureshape->filterEffectStack()->appendFilterEffect(m_filterEffect);
        
        filterConfigWdg->editFilterEffect(m_filterEffect);
        m_scrollArea->setWidget(filterConfigWdg);
        m_pictureshape->update();
        
        connect(filterConfigWdg, SIGNAL(filterChanged()), this, SLOT(filterChanged()));
//         KoFilterEffect *effect = KoFilterEffectRegistry::instance()->get(id)->createFilterEffect();
    }
    
//     m_pictureshape->filterEffectStack()->appendFilterEffect(effect);
//     m_pictureshape->update();
    
//     switch(cmbBoxIndex)
//     {
//     case 0:
//         m_pictureshape->setMode(PictureShape::Standard);
//         break;
//     
//     case 1:
//         m_pictureshape->setMode(PictureShape::Greyscale);
//         break;
//     
//     case 2:
//         m_pictureshape->setMode(PictureShape::Mono);
//         break;
//     }
}

void PictureTool::setImageData(KJob *job)
{
    if (m_pictureshape == 0)
        return; // ugh, the user deselected the image in between. We should move this code to main anyway redesign it
    KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
    Q_ASSERT(transferJob);

    if (m_pictureshape->imageCollection()) {
        KoImageData *data = m_pictureshape->imageCollection()->createImageData(transferJob->data());
        ChangeImageCommand *cmd = new ChangeImageCommand(m_pictureshape, data);
        canvas()->addCommand(cmd);
    }

}

void PictureTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    if(canvas()->shapeManager()->shapeAt(event->point) != m_pictureshape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    changeUrlPressed();
/*
    repaintSelection();
    updateSelectionHandler();
*/
}

#include <PictureTool.moc>
