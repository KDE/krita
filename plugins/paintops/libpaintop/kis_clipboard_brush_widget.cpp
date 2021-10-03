/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_clipboard_brush_widget.h"

#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QShowEvent>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QUuid>

#include <KoResourcePaths.h>

#include <kis_debug.h>
#include "kis_image.h"
#include "kis_clipboard.h"
#include "kis_paint_device.h"
#include "kis_gbr_brush.h"
#include "KisBrushServerProvider.h"
#include "kis_icon.h"
#include <KisResourceUserOperations.h>

#include <kstandardguiitem.h>

KisClipboardBrushWidget::KisClipboardBrushWidget(QWidget *parent, const QString &caption, KisImageWSP /*image*/)
    : KisWdgClipboardBrush(parent)
{
    setWindowTitle(caption);
    preview->setScaledContents(false);
    preview->setFixedSize(preview->size());
    preview->setStyleSheet("border: 2px solid #222; border-radius: 4px; padding: 5px; font: normal 10px;");


    m_rServer = KisBrushServerProvider::instance()->brushServer();

    m_brush = 0;

    m_clipboard = KisClipboard::instance();

    connect(m_clipboard, SIGNAL(clipChanged()), this, SLOT(slotClipboardContentChanged()));
    connect(colorAsmask, SIGNAL(toggled(bool)), this, SLOT(slotUpdateUseColorAsMask(bool)));
    connect(preserveAlpha, SIGNAL(toggled(bool)), this, SLOT(slotCreateBrush()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotAddPredefined()));
    connect(nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotUpdateSaveButton()));

    spacingWidget->setSpacing(true, 1.0);
    connect(spacingWidget, SIGNAL(sigSpacingChanged()), SLOT(slotSpacingChanged()));

    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Save), KStandardGuiItem::save());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
}

KisClipboardBrushWidget::~KisClipboardBrushWidget()
{
}

void KisClipboardBrushWidget::slotClipboardContentChanged()
{
    slotCreateBrush();
    if (m_brush) {
        colorAsmask->setChecked(true); // initializing this has to happen here since we need a valid brush for it to work
        preserveAlpha->setEnabled(true);
        preserveAlpha->setChecked(false);
    }
}

void KisClipboardBrushWidget::slotCreateBrush()
{
    // do nothing if it's hidden otherwise it can break the active brush is something is copied
    if (m_clipboard->hasClip() && !isHidden()) {

        pd = m_clipboard->clip(QRect(0, 0, 0, 0), false);     //Weird! Don't know how this works!
        if (pd) {
            QRect rc = pd->exactBounds();

            m_brush = KisBrushSP(new KisGbrBrush(pd, rc.x(), rc.y(), rc.width(), rc.height()));

            m_brush->setSpacing(spacingWidget->spacing());
            m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
            m_brush->setFilename(TEMPORARY_CLIPBOARD_BRUSH_FILENAME);
            m_brush->setName(TEMPORARY_CLIPBOARD_BRUSH_NAME);
            m_brush->setValid(true);

            if (colorAsmask->isChecked()) {
                static_cast<KisGbrBrush*>(m_brush.data())->makeMaskImage(preserveAlpha->isChecked());
                static_cast<KisGbrBrush*>(m_brush.data())->setBrushApplication(preserveAlpha->isChecked() ? LIGHTNESSMAP : ALPHAMASK);
            } else {
                static_cast<KisGbrBrush*>(m_brush.data())->setBrushApplication(IMAGESTAMP);
            }

            int w = preview->size().width()-10;
            preview->setPixmap(QPixmap::fromImage(m_brush->image().scaled(w, w, Qt::KeepAspectRatio)));
        }
    } else {
        preview->setText(i18n("Nothing copied\n to Clipboard"));
    }

    if (!m_brush) {
        buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    } else {
        buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    }
}

void KisClipboardBrushWidget::slotSpacingChanged()
{
    if (m_brush) {
        m_brush->setSpacing(spacingWidget->spacing());
        m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
    }
}

void KisClipboardBrushWidget::showEvent(QShowEvent *)
{
    slotClipboardContentChanged();
}

void KisClipboardBrushWidget::slotUpdateUseColorAsMask(bool useColorAsMask)
{
    preserveAlpha->setEnabled(useColorAsMask);
    slotCreateBrush();
}

void KisClipboardBrushWidget::slotAddPredefined()
{
    if(!m_brush) return;

    QString extension = ".gbr";
    QString name = nameEdit->text();

    if (m_rServer) {
        KisGbrBrushSP resource = m_brush->clone().dynamicCast<KisGbrBrush>();

        if (nameEdit->text().isEmpty()) {
            resource->setName(QUuid::createUuid().toByteArray().toHex());
        }
        else {
            resource->setName(name);
        }

        resource->setFilename(resource->name().split(" ").join("_") + extension);

        KisResourceModel model(ResourceType::Brushes);
        KisResourceUserOperations::addResourceWithUserInput(this, &model, resource);

        emit sigNewPredefinedBrush(resource);
    }

    close();
}

void KisClipboardBrushWidget::slotUpdateSaveButton()
{
    if (QFileInfo(m_rServer->saveLocation() + "/" + nameEdit->text().split(" ").join("_")
                  + ".gbr").exists()) {
        buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Overwrite"));
    } else {
        buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Save"));
    }
}

#include "moc_kis_clipboard_brush_widget.cpp"
