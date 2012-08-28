/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>
   Copyright 2011 Silvio Heinrich <plassy@web.de>

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
#include "ClipCommand.h"
#include "CropWidget.h"

#include <KLocale>
#include <KUrl>
#include <KFileDialog>
#include <KIO/Job>

#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoDocumentResourceManager.h>
#include <KoPointerEvent.h>
#include <KoFilterEffect.h>
#include <KoFilterEffectRegistry.h>
#include <KoFilterEffectConfigWidgetBase.h>
#include <KoFilterEffectStack.h>

#include "ui_wdgPictureTool.h"

struct PictureToolUI: public QWidget, public Ui::PictureTool
{
    PictureToolUI()
    {
        setupUi(this);
    }

    void blockAllSignals(bool block)
    {
        leftDoubleSpinBox->blockSignals(block);
        rightDoubleSpinBox->blockSignals(block);
        topDoubleSpinBox->blockSignals(block);
        bottomDoubleSpinBox->blockSignals(block);
        cropWidget->blockSignals(block);
        cbAspect->blockSignals(block);
        cmbColorMode->blockSignals(block);
    }
};

// ---------------------------------------------------- //

PictureTool::PictureTool(KoCanvasBase* canvas)
    : KoToolBase(canvas),
      m_pictureshape(0),
      m_pictureToolUI(0)
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

    if (m_pictureToolUI) {
        m_pictureToolUI->cropWidget->setPictureShape(m_pictureshape);
        updateControlElements();
    }

    useCursor(Qt::ArrowCursor);
}

void PictureTool::deactivate()
{
    m_pictureshape = 0;
}

QWidget *PictureTool::createOptionWidget()
{
    m_pictureToolUI = new PictureToolUI();
    m_pictureToolUI->cmbColorMode->addItem(i18n("Standard")  , PictureShape::Standard);
    m_pictureToolUI->cmbColorMode->addItem(i18n("Greyscale") , PictureShape::Greyscale);
    m_pictureToolUI->cmbColorMode->addItem(i18n("Monochrome"), PictureShape::Mono);
    m_pictureToolUI->cmbColorMode->addItem(i18n("Watermark") , PictureShape::Watermark);
    m_pictureToolUI->bnImageFile->setIcon(koIcon("document-open"));

    updateControlElements();

    connect(m_pictureToolUI->bnImageFile, SIGNAL(clicked(bool)), this, SLOT(changeUrlPressed()));
    connect(m_pictureToolUI->cmbColorMode, SIGNAL(currentIndexChanged(int)), this, SLOT(colorModeChanged(int)));
    connect(m_pictureToolUI->leftDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(cropEditFieldsChanged()));
    connect(m_pictureToolUI->rightDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(cropEditFieldsChanged()));
    connect(m_pictureToolUI->topDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(cropEditFieldsChanged()));
    connect(m_pictureToolUI->bottomDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(cropEditFieldsChanged()));
    connect(m_pictureToolUI->cbAspect, SIGNAL(toggled(bool)), this, SLOT(aspectCheckBoxChanged(bool)));
    connect(m_pictureToolUI->bnFill, SIGNAL(pressed()), this, SLOT(fillButtonPressed()));
    connect(m_pictureToolUI->cbContour, SIGNAL(toggled(bool)), this, SLOT(contourCheckBoxChanged(bool)));
    connect(m_pictureToolUI->cropWidget, SIGNAL(sigCropRegionChanged(QRectF, bool)), this, SLOT(cropRegionChanged(QRectF, bool)));

    return m_pictureToolUI;
}

void PictureTool::updateControlElements()
{
    if (m_pictureshape) {
        QSizeF imageSize = m_pictureshape->imageData()->imageSize();
        PictureShape::ColorMode mode      = m_pictureshape->colorMode();
        ClippingRect clippingRect(m_pictureshape->cropRect());

        clippingRect.right = 1.0 - clippingRect.right;
        clippingRect.bottom = 1.0 - clippingRect.bottom;
        clippingRect.scale(imageSize);

        m_pictureToolUI->blockAllSignals(true);
        m_pictureToolUI->cropWidget->setPictureShape(m_pictureshape);
        m_pictureToolUI->cropWidget->setKeepPictureProportion(m_pictureshape->isPictureInProportion());
        m_pictureToolUI->cbAspect->setChecked(m_pictureshape->isPictureInProportion());
        m_pictureToolUI->cmbColorMode->setCurrentIndex(m_pictureToolUI->cmbColorMode->findData(mode));

        m_pictureToolUI->leftDoubleSpinBox->setRange(0.0, imageSize.width());
        m_pictureToolUI->rightDoubleSpinBox->setRange(0.0, imageSize.width());
        m_pictureToolUI->topDoubleSpinBox->setRange(0.0, imageSize.height());
        m_pictureToolUI->bottomDoubleSpinBox->setRange(0.0, imageSize.height());
        m_pictureToolUI->leftDoubleSpinBox->setValue(clippingRect.left);
        m_pictureToolUI->rightDoubleSpinBox->setValue(clippingRect.right);
        m_pictureToolUI->topDoubleSpinBox->setValue(clippingRect.top);
        m_pictureToolUI->bottomDoubleSpinBox->setValue(clippingRect.bottom);
        m_pictureToolUI->cbContour->setChecked(m_pictureshape->clipPath() != 0);
        m_pictureToolUI->blockAllSignals(false);
    }
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

void PictureTool::cropEditFieldsChanged()
{
    ClippingRect clippingRect;
    clippingRect.left = m_pictureToolUI->leftDoubleSpinBox->value();
    clippingRect.right = m_pictureToolUI->rightDoubleSpinBox->value();
    clippingRect.top = m_pictureToolUI->topDoubleSpinBox->value();
    clippingRect.bottom = m_pictureToolUI->bottomDoubleSpinBox->value();
    clippingRect.uniform = false;
    clippingRect.inverted = true;
    clippingRect.normalize(m_pictureshape->imageData()->imageSize());

    m_pictureToolUI->cropWidget->setCropRect(clippingRect.toRect());
}

void PictureTool::cropRegionChanged(const QRectF& rect, bool undoPrev)
{
    if (undoPrev) {
        canvas()->shapeController()->resourceManager()->undoStack()->undo();
    }

    ChangeImageCommand *cmd = new ChangeImageCommand(m_pictureshape, rect);
    // connect before adding the command, so that "updateControlElements()" is executed
    // when the command is added to the undo stack.
    connect(cmd, SIGNAL(sigExecuted()), this, SLOT(updateControlElements()));
    canvas()->addCommand(cmd);
}

void PictureTool::colorModeChanged(int cmbIndex)
{
    PictureShape::ColorMode mode = (PictureShape::ColorMode)m_pictureToolUI->cmbColorMode->itemData(cmbIndex).toInt();
    ChangeImageCommand     *cmd  = new ChangeImageCommand(m_pictureshape, mode);
    canvas()->addCommand(cmd);
    // connect after adding the command to the undo stack to prevent a
    // call to "updateControlElements()" at this point
    connect(cmd, SIGNAL(sigExecuted()), this, SLOT(updateControlElements()));
}

void PictureTool::aspectCheckBoxChanged(bool checked)
{
    m_pictureToolUI->cropWidget->setKeepPictureProportion(checked);
}

void PictureTool::contourCheckBoxChanged(bool checked)
{
    canvas()->addCommand(new ClipCommand(m_pictureshape, checked));
}

void PictureTool::fillButtonPressed()
{
    m_pictureToolUI->cropWidget->maximizeCroppedArea();
}

void PictureTool::setImageData(KJob *job)
{
    if (m_pictureshape == 0)
        return; // ugh, the user deselected the image in between. We should move this code to main anyway redesign it

    KIO::StoredTransferJob *transferJob = qobject_cast<KIO::StoredTransferJob*>(job);
    Q_ASSERT(transferJob);
    if (m_pictureshape->imageCollection()) {
        KoImageData        *data = m_pictureshape->imageCollection()->createImageData(transferJob->data());
        ChangeImageCommand *cmd  = new ChangeImageCommand(m_pictureshape, data);
        // connect before adding the command, so that "updateControlElements()" is executed
        // when the command is added to the undo stack.
        connect(cmd, SIGNAL(sigExecuted()), this, SLOT(updateControlElements()));
        canvas()->addCommand(cmd);
    }
}

void PictureTool::mousePressEvent(KoPointerEvent *event)
{
    if (event->button() == Qt::RightButton) {
        event->ignore();
    }
}

void PictureTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != m_pictureshape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    changeUrlPressed();
}
