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

#include <KoIcon.h>

#include <klocalizedstring.h>


DlgCanvasSize::DlgCanvasSize(QWidget *parent, int width, int height)
        : KDialog(parent),
          m_originalWidth(width),
          m_originalHeight(height),
          m_aspectRatio((double)width / height),
          m_keepAspect(true),
          m_newWidth(width),
          m_newHeight(height),
          m_xOffset(0),
          m_yOffset(0)
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

    m_group = new QButtonGroup(m_page);
    m_group->addButton(m_page->topLeft, NORTH_WEST);
    m_group->addButton(m_page->topCenter, NORTH);
    m_group->addButton(m_page->topRight, NORTH_EAST);

    m_group->addButton(m_page->middleLeft, WEST);
    m_group->addButton(m_page->middleCenter, CENTER);
    m_group->addButton(m_page->middleRight, EAST);

    m_group->addButton(m_page->bottomLeft, SOUTH_WEST);
    m_group->addButton(m_page->bottomCenter, SOUTH);
    m_group->addButton(m_page->bottomRight, SOUTH_EAST);

    connect(m_group, SIGNAL(buttonClicked(int)), SLOT(slotAnchorButtonClicked(int)));
    connect(m_page->comboUnit, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotUpdateSizeTextBoxes()));
    connect(m_page->aspectRatio, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedXOffset(int)), m_page->xOffset, SLOT(setValue(int)));
    connect(m_page->canvasPreview, SIGNAL(sigModifiedYOffset(int)), m_page->yOffset, SLOT(setValue(int)));

    m_page->newWidth->setValue(width);
    m_page->newHeight->setValue(height);

    m_page->canvasPreview->setImageSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setCanvasSize(m_originalWidth, m_originalHeight);
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);

    setMainWidget(m_page);
    loadAnchorIcons();
    m_group->button(NORTH_WEST)->setChecked(true);
    updateAnchorIcons(NORTH_WEST);
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
    QString index = m_page->comboUnit->currentText();

    m_newWidth = v;
    if (index == i18n("Percent")) {
        m_newWidth = m_page->newWidth->value() / 100.0f * m_originalWidth;
    }

    if (m_keepAspect) {
        m_newHeight = (qint32)qRound(m_newWidth / m_aspectRatio);
    }

    int savedId = m_group->checkedId();

    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
    slotUpdateSizeTextBoxes();
    updateOffset(savedId);
    updateButtons(savedId);
}

void DlgCanvasSize::slotHeightChanged(int v)
{
    QString index = m_page->comboUnit->currentText();

    m_newHeight = v;
    if (index == i18n("Percent")) {
        m_newHeight = m_page->newHeight->value() / 100.0f * m_originalHeight;
    }

    if (m_keepAspect) {
        m_newWidth = (qint32)qRound(m_newHeight * m_aspectRatio);
    }

    int savedId = m_group->checkedId();

    m_page->canvasPreview->setCanvasSize(m_newWidth, m_newHeight);
    slotUpdateSizeTextBoxes();
    updateOffset(savedId);
    updateButtons(savedId);
}

void DlgCanvasSize::slotXOffsetChanged(int v)
{
    m_xOffset = v;
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
    updateButtons(-1);
}

void DlgCanvasSize::slotYOffsetChanged(int v)
{
    m_yOffset = v;
    m_page->canvasPreview->setImageOffset(m_xOffset, m_yOffset);
    updateButtons(-1);
}

void DlgCanvasSize::slotAnchorButtonClicked(int id)
{
    updateOffset(id);
    updateButtons(id);
}

void DlgCanvasSize::updateButtons(int forceId)
{
    int id = m_group->checkedId();

    if (forceId != -1) {
        m_group->setExclusive(true);
        m_group->button(forceId)->setChecked(true);
        updateAnchorIcons(forceId);
    } else if (id != -1) {
        int xOffset, yOffset;
        expectedOffset(id, xOffset, yOffset);

        bool offsetAsExpected =
            xOffset == m_xOffset &&
            yOffset == m_yOffset;

        if (offsetAsExpected) {
            m_group->setExclusive(true);
        } else {
            m_group->setExclusive(false);
            m_group->button(id)->setChecked(false);
            id = -1;
        }

        updateAnchorIcons(id);
    } else {
        updateAnchorIcons(id);
    }
}

void DlgCanvasSize::expectedOffset(int id, int &xOffset, int &yOffset)
{
    qreal xCoeff = (id % 3) * 0.5;
    qreal yCoeff = (id / 3) * 0.5;

    int xDiff = m_newWidth - m_originalWidth;
    int yDiff = m_newHeight - m_originalHeight;

    xOffset = xDiff * xCoeff;
    yOffset = yDiff * yCoeff;
}

void DlgCanvasSize::updateOffset(int id)
{
    if (id == -1) return;

    int xOffset;
    int yOffset;
    expectedOffset(id, xOffset, yOffset);

    m_page->xOffset->setValue(xOffset);
    m_page->yOffset->setValue(yOffset);
}

void DlgCanvasSize::slotUpdateSizeTextBoxes()
{
    QString index = m_page->comboUnit->currentText();
    m_page->newWidth->blockSignals(true);
    m_page->newHeight->blockSignals(true);

    if (index == i18n("Pixels")) {
        m_page->newWidth->setSuffix(QString());
        m_page->newWidth->setValue(m_newWidth);
        m_page->newHeight->setSuffix(QString());
        m_page->newHeight->setValue(m_newHeight);
    } else if (index == i18n("Percent")) {
        m_page->newWidth->setSuffix(QString("%"));
        m_page->newWidth->setValue(qRound((float)m_newWidth / m_originalWidth * 100));
        m_page->newHeight->setSuffix(QString("%"));
        m_page->newHeight->setValue(qRound((float)m_newHeight / m_originalHeight * 100));
    }

    m_page->xOffset->setMinimum(-m_newWidth + 1);
    m_page->xOffset->setMaximum(m_newWidth - 1);
    m_page->yOffset->setMinimum(-m_newHeight + 1);
    m_page->yOffset->setMaximum(m_newHeight - 1);

    m_page->newWidth->blockSignals(false);
    m_page->newHeight->blockSignals(false);
}

void DlgCanvasSize::loadAnchorIcons()
{
    m_anchorIcons[NORTH_WEST] =  koIcon("arrow_north_west");
    m_anchorIcons[NORTH] = koIcon("arrow_north");
    m_anchorIcons[NORTH_EAST] = koIcon("arrow_north_east");
    m_anchorIcons[EAST] = koIcon("arrow_east");
    m_anchorIcons[CENTER] = koIconWanted("though currently m_anchorIcons[CENTER] is not used","arrow_center");
    m_anchorIcons[WEST] = koIcon("arrow_west");
    m_anchorIcons[SOUTH_WEST] = koIcon("arrow_south_west");
    m_anchorIcons[SOUTH] = koIcon("arrow_south");
    m_anchorIcons[SOUTH_EAST] = koIcon("arrow_south_east");
}

void DlgCanvasSize::updateAnchorIcons(int id)
{
    anchor iconLayout[10][9] = {
        {NONE, EAST, NONE, SOUTH, SOUTH_EAST, NONE, NONE, NONE, NONE},
        {WEST, NONE, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, NONE, NONE, NONE},
        {NONE, WEST, NONE, NONE, SOUTH_WEST, SOUTH, NONE, NONE, NONE},
        {NORTH, NORTH_EAST, NONE, NONE, EAST, NONE, SOUTH, SOUTH_EAST, NONE},
        {NORTH_WEST, NORTH, NORTH_EAST, WEST, NONE, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST},
        {NONE, NORTH_WEST, NORTH, NONE, WEST, NONE, NONE, SOUTH_WEST, SOUTH},
        {NONE, NONE, NONE, NORTH, NORTH_EAST, NONE, NONE, EAST, NONE},
        {NONE, NONE, NONE, NORTH_WEST, NORTH, NORTH_EAST, WEST, NONE, EAST},
        {NONE, NONE, NONE, NONE, NORTH_WEST, NORTH, NONE, WEST, NONE},
        {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE}
    };

    if (id == -1) {
        id = SOUTH_EAST + 1;
    }

    for (int i = NORTH_WEST; i <= SOUTH_EAST; i++) {
        anchor iconId = iconLayout[id][i];
        QAbstractButton *button = m_group->button(i);

        if (iconId == NONE) {
            button->setIcon(KIcon());
        } else {
            button->setIcon(m_anchorIcons[iconId]);
        }
    }

}
