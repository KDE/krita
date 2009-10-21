/*
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dlg_canvassize.h"
#include "kcanvaspreview.h"

#include <klocalizedstring.h>

#include <math.h>


DlgCanvasSize::DlgCanvasSize(QWidget *parent, int width, int height)
        : KDialog(parent), m_originalWidth(width), m_originalHeight(height), m_aspectRatio((double)width / height), m_keepAspect(true)
{
    setCaption(i18n("Canvas Size"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgCanvasSize(this);
    m_page->layout()->setMargin(0);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("canvas_size");

    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->newWidth, SIGNAL(valueChanged(int)), this, SLOT(slotWidthChanged(int)));
    connect(m_page->newHeight, SIGNAL(valueChanged(int)), this, SLOT(slotHeightChanged(int)));

    connect(m_page->xOffset, SIGNAL(valueChanged(int)), this, SLOT(slotXOffsetChanged(int)));
    connect(m_page->yOffset, SIGNAL(valueChanged(int)), this, SLOT(slotYOffsetChanged(int)));

    connect(m_page->topLeft, SIGNAL(clicked()), this, SLOT(slotTopLeftClicked()));
    connect(m_page->topCenter, SIGNAL(clicked()), this, SLOT(slotTopCenterClicked()));
    connect(m_page->topRight, SIGNAL(clicked()), this, SLOT(slotTopRightClicked()));
    connect(m_page->middleLeft, SIGNAL(clicked()), this, SLOT(slotMiddleLeftClicked()));
    connect(m_page->middleCenter, SIGNAL(clicked()), this, SLOT(slotMiddleCenterClicked()));
    connect(m_page->middleRight, SIGNAL(clicked()), this, SLOT(slotMiddleRightClicked()));
    connect(m_page->bottomLeft, SIGNAL(clicked()), this, SLOT(slotBottomLeftClicked()));
    connect(m_page->bottomCenter, SIGNAL(clicked()), this, SLOT(slotBottomCenterClicked()));
    connect(m_page->bottomRight, SIGNAL(clicked()), this, SLOT(slotBottomRightClicked()));

    connect(m_page->comboWidthUnit, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotWidthUnitChanged(QString)));
    connect(m_page->comboHeightUnit, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotHeightUnitChanged(QString)));

    connect(m_page->aspectRatio, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_page->canvasPreview, SIGNAL(sigModifiedXOffset(int)), m_page->xOffset, SLOT(setValue(int)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedYOffset(int)), m_page->yOffset, SLOT(setValue(int)));

    m_page->xOffset->setMinimum(-width + 1);
    m_page->xOffset->setMaximum(width - 1);
    m_page->yOffset->setMinimum(-height + 1);
    m_page->yOffset->setMaximum(height - 1);

    m_page->newWidth->setValue(width);
    m_page->newHeight->setValue(height);

    m_page->canvasPreview->setImageSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setCanvasSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setImageOffset(0, 0);

    loadAnchorIcons();
    updateAnchorIcons(CENTER);

    setMainWidget(m_page);
}

DlgCanvasSize::~DlgCanvasSize()
{
    delete m_page;
}

qint32 DlgCanvasSize::width()
{
    return (qint32)m_newWidth;
}

qint32 DlgCanvasSize::height()
{
    return (qint32)m_newHeight;
}

qint32 DlgCanvasSize::xOffset()
{
    return (qint32)m_page->xOffset->value();
}

qint32 DlgCanvasSize::yOffset()
{
    return (qint32)m_page->yOffset->value();
}

void DlgCanvasSize::slotAspectChanged(bool keep)
{
    m_keepAspect = keep;
}

void DlgCanvasSize::slotWidthChanged(int v)
{
    QString index = m_page->comboWidthUnit->currentText();

    m_newWidth = v;
    if (index == i18n("Percent"))
        m_newWidth = m_page->newWidth->value() / 100.0f * m_originalWidth;

    m_page->xOffset->setMaximum(m_newWidth - 1);

    if (m_keepAspect) {
        m_newHeight = (qint32)round(m_newWidth / m_aspectRatio);

        m_page->yOffset->setMaximum(m_newHeight - 1);

        m_page->newHeight->blockSignals(true);
        slotHeightUnitChanged(QString());
        m_page->newHeight->blockSignals(false);
    }

    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
}

void DlgCanvasSize::slotHeightChanged(int v)
{
    QString index = m_page->comboWidthUnit->currentText();

    m_newHeight = v;
    if (index == i18n("Percent"))
        m_newHeight = m_page->newHeight->value() / 100.0f * m_originalHeight;

    if (m_keepAspect) {
        m_newWidth = (qint32)round(m_newHeight * m_aspectRatio);

        m_page->xOffset->setMaximum(m_newWidth - 1);

        m_page->newWidth->blockSignals(true);
        slotWidthUnitChanged(QString());
        m_page->newWidth->blockSignals(false);
    }

    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
}

void DlgCanvasSize::slotXOffsetChanged(int v)
{
    m_xOffset = v;
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
}

void DlgCanvasSize::slotYOffsetChanged(int v)
{
    m_yOffset = v;
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
}

void DlgCanvasSize::slotTopLeftClicked()
{
    m_page->xOffset->setValue(0);
    m_page->yOffset->setValue(0);

    updateAnchorIcons(NORTH_WEST);
}

void DlgCanvasSize::slotTopCenterClicked()
{
    m_page->xOffset->setValue((int)((m_newWidth - m_originalWidth) / 2.0));
    m_page->yOffset->setValue(0);

    updateAnchorIcons(NORTH);
}

void DlgCanvasSize::slotTopRightClicked()
{
    m_page->xOffset->setValue(m_newWidth - m_originalWidth);
    m_page->yOffset->setValue(0);

    updateAnchorIcons(NORTH_EAST);
}

void DlgCanvasSize::slotMiddleLeftClicked()
{
    m_page->xOffset->setValue(0);
    m_page->yOffset->setValue((int)((m_newHeight - m_originalHeight) / 2.0));

    updateAnchorIcons(WEST);
}

void DlgCanvasSize::slotMiddleCenterClicked()
{
    m_page->xOffset->setValue((int)(m_newWidth - m_originalWidth) / 2.0);
    m_page->yOffset->setValue((int)((m_newHeight - m_originalHeight) / 2.0));

    updateAnchorIcons(CENTER);
}

void DlgCanvasSize::slotMiddleRightClicked()
{
    m_page->xOffset->setValue(m_newWidth - m_originalWidth);
    m_page->yOffset->setValue((int)((m_newHeight - m_originalHeight) / 2.0));

    updateAnchorIcons(EAST);
}

void DlgCanvasSize::slotBottomLeftClicked()
{
    m_page->xOffset->setValue(0);
    m_page->yOffset->setValue(m_newHeight - m_originalHeight);

    updateAnchorIcons(SOUTH_WEST);
}

void DlgCanvasSize::slotBottomCenterClicked()
{
    m_page->xOffset->setValue((int)(m_newWidth - m_originalWidth) / 2.0);
    m_page->yOffset->setValue(m_newHeight - m_originalHeight);

    updateAnchorIcons(SOUTH);
}

void DlgCanvasSize::slotBottomRightClicked()
{
    m_page->xOffset->setValue(m_newWidth - m_originalWidth);
    m_page->yOffset->setValue(m_newHeight - m_originalHeight);

    updateAnchorIcons(SOUTH_EAST);
}

void DlgCanvasSize::slotWidthUnitChanged(QString)
{
    QString index = m_page->comboWidthUnit->currentText();
    m_page->newWidth->blockSignals(true);

    if (index == i18n("Pixels")) {
        m_page->newWidth->setSuffix(QString());
        m_page->newWidth->setValue(m_newWidth);
    } else if (index == i18n("Percent")) {
        m_page->newWidth->setSuffix(QString("%"));
        m_page->newWidth->setValue(round((float)m_newWidth / m_originalWidth * 100));
    }

    m_page->newWidth->blockSignals(false);
}

void DlgCanvasSize::slotHeightUnitChanged(QString)
{
    QString index = m_page->comboHeightUnit->currentText();
    m_page->newHeight->blockSignals(true);

    if (index == QString("Pixels")) {
        m_page->newHeight->setSuffix(QString());
        m_page->newHeight->setValue(m_newHeight);
    } else if (index == QString("Percent")) {
        m_page->newHeight->setSuffix(QString("%"));
        m_page->newHeight->setValue(round((float)m_newHeight / m_originalHeight * 100));
    }

    m_page->newHeight->blockSignals(false);
}

void DlgCanvasSize::loadAnchorIcons()
{
    m_anchorIcons[NORTH_WEST] =  KIcon("arrow_north_west.png");
    m_anchorIcons[NORTH] = KIcon("arrow_north.png");
    m_anchorIcons[NORTH_EAST] = KIcon("arrow_north_east.png");
    m_anchorIcons[EAST] = KIcon("arrow_east.png");
    m_anchorIcons[CENTER] = KIcon("arrow_center.png");
    m_anchorIcons[WEST] = KIcon("arrow_west.png");
    m_anchorIcons[SOUTH_WEST] = KIcon("arrow_south_west.png");
    m_anchorIcons[SOUTH] = KIcon("arrow_south.png");
    m_anchorIcons[SOUTH_EAST] = KIcon("arrow_south_east.png");
}

void DlgCanvasSize::updateAnchorIcons(anchor enumAnchor)
{
    anchor iconLayout[9][9] = { {NONE, EAST, NONE, SOUTH, SOUTH_EAST, NONE, NONE, NONE, NONE},
        {WEST, NONE, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, NONE, NONE, NONE},
        {NONE, WEST, NONE, NONE, SOUTH_WEST, SOUTH, NONE, NONE, NONE},
        {NORTH, NORTH_EAST, NONE, NONE, EAST, NONE, SOUTH, SOUTH_EAST, NONE},
        {NORTH_WEST, NORTH, NORTH_EAST, WEST, NONE, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST},
        {NONE, NORTH_WEST, NORTH, NONE, WEST, NONE, NONE, SOUTH_WEST, SOUTH},
        {NONE, NONE, NONE, NORTH, NORTH_EAST, NONE, NONE, EAST, NONE},
        {NONE, NONE, NONE, NORTH_WEST, NORTH, NORTH_EAST, WEST, NONE, EAST},
        {NONE, NONE, NONE, NONE, NORTH_WEST, NORTH, NONE, WEST, NONE}
    };

    setButtonIcon(m_page->topLeft, iconLayout[enumAnchor][NORTH_WEST]);
    setButtonIcon(m_page->topCenter, iconLayout[enumAnchor][NORTH]);
    setButtonIcon(m_page->topRight, iconLayout[enumAnchor][NORTH_EAST]);
    setButtonIcon(m_page->middleLeft, iconLayout[enumAnchor][WEST]);
    setButtonIcon(m_page->middleCenter, iconLayout[enumAnchor][CENTER]);
    setButtonIcon(m_page->middleRight, iconLayout[enumAnchor][EAST]);
    setButtonIcon(m_page->bottomLeft, iconLayout[enumAnchor][SOUTH_WEST]);
    setButtonIcon(m_page->bottomCenter, iconLayout[enumAnchor][SOUTH]);
    setButtonIcon(m_page->bottomRight, iconLayout[enumAnchor][SOUTH_EAST]);
}

void DlgCanvasSize::setButtonIcon(QPushButton *button, anchor enumAnchorIcon)
{
    if (enumAnchorIcon == NONE) {
        button->setIcon(KIcon());
    } else {
        button->setIcon(m_anchorIcons[enumAnchorIcon]);
    }
}

#include "dlg_canvassize.moc"
