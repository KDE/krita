/*
 *  SPDX-FileCopyrightText: 2017 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_wdg_options_heightmap.h"

#include <QtMath>
#include <QToolTip>

#include <kis_assert.h>
#include <kis_paint_layer.h>

static constexpr quint32 nextPow2(quint32 n)
{
    return ((((((n - 1) | n >> 1) | n >> 2) | n >> 4) | n >> 8) | n >> 16) + 1 + (quint32)(n == 0);
}

static constexpr quint32 prevPow2(quint32 n)
{
    return nextPow2(n) >> 1;
}

static void showErrorTooltip(const QString &msg)
{
    QToolTip::showText(QCursor::pos(), i18n("Error: ") + msg);
}

KisWdgOptionsHeightmap::KisWdgOptionsHeightmap(QWidget *parent)
    : KisWdgOptionsHeightmap(parent, false)
{
}

KisWdgOptionsHeightmap::KisWdgOptionsHeightmap(QWidget *parent, bool export_mode)
    : KisConfigWidget(parent)
    , m_exportMode(export_mode)
{
    setupUi(this);

    if (m_exportMode) {
        dimensionsGroupBox->setVisible(false);
        fileSizeDescLabel->setVisible(false);
        fileSizeLabel->setVisible(false);
        bppDescLabel->setVisible(false);
        bppLabel->setVisible(false);
    }
    else {
        connect(guessButton, SIGNAL(clicked(bool)), this, SLOT(guessDimensions()));
        connect(widthInput, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));
        connect(heightInput, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)));
    }
}

void KisWdgOptionsHeightmap::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    int endianness = cfg->getInt("endianness", 1);
    if (endianness == 0) {
        radioBig->setChecked(true);
    }
    else {
        radioLittle->setChecked(true);
    }
}

KisPropertiesConfigurationSP KisWdgOptionsHeightmap::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    if (radioBig->isChecked()) {
        cfg->setProperty("endianness", 0);
    }
    else {
        cfg->setProperty("endianness", 1);
    }
    return cfg;
}

void KisWdgOptionsHeightmap::showEvent(QShowEvent *event)
{
    updateStatus();
    QWidget::showEvent(event);
}

void KisWdgOptionsHeightmap::updateStatus()
{
    if (m_exportMode)
        return;
    bool ok;
    int fileSize = fileSizeLabel->text().toInt(&ok);
    KIS_ASSERT_RECOVER_RETURN(ok);
    KIS_ASSERT_RECOVER_RETURN(fileSize > 0);
    int w = widthInput->value();
    int h = heightInput->value();

    quint32 depth = bppLabel->text().toUInt(&ok);
    KIS_ASSERT_RECOVER_RETURN(ok);

    QString old_status = statusLabel->text();

    int bytesPerPixel = depth / 8;
    int dataSize = w * h * bytesPerPixel;

    bool status_ok = false;

    QString fileMismatch = i18n("Input does not match file size");
    if (w == 0 && h == 0) {
        statusLabel->setText(i18n("Please specify width and height"));
    }
    else if (w == 0) {
        statusLabel->setText(i18n("Please specify width"));
    }
    else if (h == 0) {
        statusLabel->setText(i18n("Please specify height"));
    }
    else if (dataSize != fileSize) {
        statusLabel->setText(fileMismatch);
    }
    else {
        statusLabel->setText("");
        status_ok = true;
    }

    if (old_status.compare(statusLabel->text()) != 0) {
        emit statusUpdated(status_ok);
    }
}

void KisWdgOptionsHeightmap::widthChanged(int i)
{
    Q_UNUSED(i);
    updateStatus();
}

void KisWdgOptionsHeightmap::heightChanged(int i)
{
    Q_UNUSED(i);
    updateStatus();
}

void KisWdgOptionsHeightmap::guessDimensions()
{
    quint32 w = widthInput->value();
    quint32 h = heightInput->value();

    bool ok;
    quint32 fileSize = fileSizeLabel->text().toUInt(&ok);
    KIS_ASSERT_RECOVER_RETURN(ok);

    quint32 depth = bppLabel->text().toUInt(&ok);
    KIS_ASSERT_RECOVER_RETURN(ok);

    quint32 bytesPerPixel = depth / 8;

    w = widthInput->text().toUInt(&ok);
    KIS_ASSERT_RECOVER_RETURN(ok);

    h = heightInput->text().toUInt(&ok);
    KIS_ASSERT_RECOVER_RETURN(ok);

    quint32 dimensions = fileSize / bytesPerPixel;

    if (w > 0 && h > 0) {
        if (w * h == dimensions) {
            // toggle landscape/portrait orientation
            widthInput->setValue(h);
            heightInput->setValue(w);
        }
    }
    else if (w == 0 && h == 0) {
        quint32 r = (quint32)(qFloor(qSqrt(dimensions) + 0.5));

        // First guess, square image?
        if (r*r == dimensions) {
            widthInput->setValue(r);
            heightInput->setValue(r);
        }
        else {
            // second guess, power of two?
            w = prevPow2(r);
            h = dimensions / w + (dimensions % w);
            if (w * h != dimensions) {
                showErrorTooltip(i18n("Too many possible combinations. Input a width or height and try again."));
                return;
            }

            // prefer landscape orientation
            widthInput->setValue(w > h ? w : h);
            heightInput->setValue(w > h ? h : w);

            // TODO: cycle through other pow2 combinations if called multiple times?
        }
    }
    else if (w > 0) {
        if (w > dimensions) {
            showErrorTooltip(i18n("Width exceeds available pixels."));
            return;
        }
        h = dimensions / w + (dimensions % w);
        if (w * h != dimensions) {
            showErrorTooltip(i18n("Unable to calculate an appropriate height. File does not contain enough pixels to form a rectangle."));
            return;
        }
        heightInput->setValue(h);
    }
    else {
        if (h > dimensions) {
            showErrorTooltip(i18n("Height exceeds available pixels."));
            return;
        }
        w = dimensions / h + (dimensions % h);
        if (w * h != dimensions) {
            showErrorTooltip(i18n("Unable to calculate an appropriate width. File does not contain enough pixels to form a rectangle."));
            return;
        }
        widthInput->setValue(w);
    }
}
