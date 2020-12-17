/*
 *  dlg_imagesplit.cc - part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_imagesplit.h"

#include <klocalizedstring.h>
#include <kis_debug.h>

#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <QPainter>

DlgImagesplit::DlgImagesplit(KisViewManager* view
                             , const QString &suffix
                             , QStringList listMimeFilter
                             , int defaultMimeIndex, QImage thumbnail
                             , QList<qreal> hGuides
                             , QList<qreal> vGuides, qreal thumbnailRatio)
    : KoDialog(view->mainWindow())
{
    m_page = new WdgImagesplit(this);
    m_thumbnail = thumbnail;
    m_horizontalGuides = hGuides;
    m_verticalGuides = vGuides;
    m_thumbnailRatio = thumbnailRatio;

    setCaption(i18n("Image Split"));
    setButtons(Apply | Close);
    setDefaultButton(Apply);

    connect(this, SIGNAL(applyClicked()), this, SLOT(applyClicked()));

    setMainWidget(m_page);
    m_page->lineEdit->setText(suffix);
    m_page->cmbFileType->clear();
    m_page->cmbFileType->addItems(listMimeFilter);
    m_page->cmbFileType->setCurrentIndex(defaultMimeIndex);
    cmbIndex = defaultMimeIndex;

    connect(m_page->chkAutoSave, SIGNAL(stateChanged(int)), SLOT(lineEditEnable()));
    connect(m_page->cmbFileType, SIGNAL(activated(int)), this, SLOT(setMimeType(int)));
    connect(m_page->chkGuidesHorizontal, SIGNAL(toggled(bool)), m_page->intHorizontalSplitLines, SLOT(setDisabled(bool)));
    connect(m_page->chkGuidesVertical, SIGNAL(toggled(bool)), m_page->intVerticalSplitLines, SLOT(setDisabled(bool)));

    connect(m_page->intVerticalSplitLines, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
    connect(m_page->intHorizontalSplitLines, SIGNAL(valueChanged(int)), SLOT(updatePreview()));
    connect(m_page->chkGuidesHorizontal, SIGNAL(toggled(bool)), SLOT(updatePreview()));
    connect(m_page->chkGuidesVertical, SIGNAL(toggled(bool)), SLOT(updatePreview()));

    updatePreview();
}

DlgImagesplit::~DlgImagesplit()
{
}

void DlgImagesplit::lineEditEnable()
{
    if (m_page->chkAutoSave->isChecked()) {
        m_page->lblSuffix->setEnabled(true);
        m_page->lineEdit->setEnabled(true);
        m_page->lblFileType->setEnabled(true);
        m_page->cmbFileType->setEnabled(true);
    }
    else
    {
        m_page->lblSuffix->setEnabled(false);
        m_page->lineEdit->setEnabled(false);
        m_page->lblFileType->setEnabled(false);
        m_page->cmbFileType->setEnabled(false);
    }

}

bool DlgImagesplit::autoSave()
{
    if (m_page->chkAutoSave->isChecked())
        return true;
    else
        return false;
}

bool DlgImagesplit::sortHorizontal()
{
    if (m_page->chkHorizontal->isChecked())
        return true;
    else
        return false;
}

int DlgImagesplit::horizontalLines()
{
    return m_page->intHorizontalSplitLines->value();
}

int DlgImagesplit::verticalLines()
{
    return m_page->intVerticalSplitLines->value();
}

bool DlgImagesplit::useHorizontalGuides()
{
    return m_page->chkGuidesHorizontal->isChecked();
}

bool DlgImagesplit::useVerticalGuides()
{
    return m_page->chkGuidesVertical->isChecked();
}

QString DlgImagesplit::suffix()
{
    return m_page->lineEdit->text();
}

void DlgImagesplit::setMimeType(int index)
{
    cmbIndex = index;
}

void DlgImagesplit::updatePreview()
{
    QImage img = QImage(200, 200, QImage::Format_RGBA8888);
    QPainter painter(&img);

    img.fill(palette().background().color());
    QPoint point;
    point.setX((img.width()-m_thumbnail.width())*0.5);
    point.setY((img.height()-m_thumbnail.height())*0.5);

    painter.setOpacity(0.5);
    painter.drawImage(point, m_thumbnail);

    painter.setOpacity(1.0);
    QPen pen = QPen(palette().highlight().color());
    pen.setWidth(1);
    painter.setPen(pen);

    if (useHorizontalGuides()) {
        for (int i = 0; i< m_horizontalGuides.size(); i++) {
            int lineY = point.y() + (m_thumbnailRatio *m_horizontalGuides[i]);
            painter.drawLine(point.x(), lineY, point.x()+m_thumbnail.width(), lineY);
        }
    } else {
        int rowHeight = m_thumbnail.height()/(horizontalLines()+1);
        for (int i = 0; i< horizontalLines(); i++) {
            int lineY = point.y()+(rowHeight*(i+1));
            painter.drawLine(point.x(), lineY, point.x()+m_thumbnail.width(), lineY);
        }
    }

    if (useVerticalGuides()) {
        for (int i = 0; i< m_verticalGuides.size(); i++) {
            int lineX = point.x() + (m_thumbnailRatio *m_verticalGuides[i]);
            painter.drawLine(lineX, point.y(), lineX, point.y()+m_thumbnail.height());
        }
    } else {
        int columnWidth = m_thumbnail.width()/(verticalLines()+1);
        for (int i = 0; i< verticalLines(); i++) {
            int lineX = point.x()+(columnWidth*(i+1));
            painter.drawLine(lineX, point.y(), lineX, point.y()+m_thumbnail.height());
        }
    }

    m_page->imagePreviewLabel->setPixmap(QPixmap::fromImage(img));
}

void DlgImagesplit::applyClicked()
{
    accept();
}

