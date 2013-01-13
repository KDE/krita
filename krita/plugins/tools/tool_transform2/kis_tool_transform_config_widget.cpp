/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_transform_config_widget.h"

#include <KoIcon.h>
#include "kis_canvas2.h"


KisToolTransformConfigWidget::KisToolTransformConfigWidget(const TransformTransactionProperties *transaction, KisCanvas2 *canvas, QWidget *parent)
    : QWidget(parent),
      m_transaction(transaction)
{
    setupUi(this);
    showDecorationsBox->setIcon(koIcon("krita_tool_transform"));
    label_shearX->setPixmap(koIcon("shear_horizontal").pixmap(16, 16));
    label_shearY->setPixmap(koIcon("shear_vertical").pixmap(16, 16));

    label_width->setPixmap(koIcon("width_icon").pixmap(16, 16));
    label_height->setPixmap(koIcon("height_icon").pixmap(16, 16));

    label_offsetX->setPixmap(koIcon("offset_horizontal").pixmap(16, 16));
    label_offsetY->setPixmap(koIcon("offset_vertical").pixmap(16, 16));
/*
    // Init labels
    QPixmap rotateX_Pixmap, rotateY_Pixmap, rotateZ_Pixmap;
    rotateX_Pixmap.loadFromData(rotateX_PNG, rotateX_PNG_len, "png");
    rotateY_Pixmap.loadFromData(rotateY_PNG, rotateY_PNG_len, "png");
    rotateZ_Pixmap.loadFromData(rotateZ_PNG, rotateZ_PNG_len, "png");
    m_optWidget->label_rotateX->setPixmap(rotateX_Pixmap);
    m_optWidget->label_rotateY->setPixmap(rotateY_Pixmap);
    m_optWidget->label_rotateZ->setPixmap(rotateZ_Pixmap);

    // Init Filter  combo
    cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    cmbFilter->setCurrent("Bicubic");
    connect(cmbFilter, SIGNAL(activated(const KoID &)),
            this, SLOT(slotFilterChanged(const KoID &)));

    // Init Warp Type combo
    cmbWarpType->insertItem(KisWarpTransformWorker::AFFINE_TRANSFORM,i18n("Affine"));
    cmbWarpType->insertItem(KisWarpTransformWorker::SIMILITUDE_TRANSFORM,i18n("Similitude"));
    cmbWarpType->insertItem(KisWarpTransformWorker::RIGID_TRANSFORM,i18n("Rigid"));
    cmbWarpType->setCurrentIndex(KisWarpTransformWorker::RIGID_TRANSFORM);
    connect(cmbWarpType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWarpTypeChanged(int)));

    // Init Rotation Center buttons
    m_handleDir[0] = QPointF(1, 0);
    m_handleDir[1] = QPointF(1, -1);
    m_handleDir[2] = QPointF(0, -1);
    m_handleDir[3] = QPointF(-1, -1);
    m_handleDir[4] = QPointF(-1, 0);
    m_handleDir[5] = QPointF(-1, 1);
    m_handleDir[6] = QPointF(0, 1);
    m_handleDir[7] = QPointF(1, 1);
    m_handleDir[8] = QPointF(0, 0); // also add the center

    m_rotationCenterButtons = new QButtonGroup(0);
    // we set the ids to match m_handleDir
    m_rotationCenterButtons->addButton(middleRightButton, 0);
    m_rotationCenterButtons->addButton(topRightButton, 1);
    m_rotationCenterButtons->addButton(middleTopButton, 2);
    m_rotationCenterButtons->addButton(topLeftButton, 3);
    m_rotationCenterButtons->addButton(middleLeftButton, 4);
    m_rotationCenterButtons->addButton(bottomLeftButton, 5);
    m_rotationCenterButtons->addButton(middleBottomButton, 6);
    m_rotationCenterButtons->addButton(bottomRightButton, 7);
    m_rotationCenterButtons->addButton(centerButton, 8);

    QToolButton *nothingSelected = new QToolButton(0);
    nothingSelected->setCheckable(true);
    nothingSelected->setAutoExclusive(true);
    nothingSelected->hide(); // a convenient button for when no button is checked in the group
    m_rotationCenterButtons->addButton(nothingSelected, 9);

    connect(m_rotationCenterButtons, SIGNAL(buttonPressed(int)), this, SLOT(slotRotationCenterChanged(int)));

    // Init Free Transform Values
    connect(scaleXBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetScaleX(qreal)));
    connect(scaleYBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetScaleY(qreal)));
    connect(shearXBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetShearX(qreal)));
    connect(shearYBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetShearY(qreal)));
    connect(translateXBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetTranslateX(qreal)));
    connect(translateYBox, SIGNAL(valueChanged(qreal)), this, SLOT(setTranslateY(qreal)));
    connect(aXBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetAX(qreal)));
    connect(aYBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetAY(qreal)));
    connect(aZBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetAZ(qreal)));
    connect(aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotSetKeepAspectRatio(bool)));

    // Init Wrap Transform Values
    connect(alphaBox, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetWrapAlpha(qreal)));
    connect(densityBox, SIGNAL(valueChanged(int)), this, SLOT(slotSetWrapDensity(int)));

    connect(defaultRadioButton, SIGNAL(clicked(bool)), this, SLOT(slotWarpDefaultPointsButtonClicked(bool)));
    connect(customRadioButton, SIGNAL(clicked(bool)), this, SLOT(slotWarpCustomPointsButtonClicked(bool)));
    connect(lockUnlockPointsButton, SIGNAL(clicked()), this, SLOT(slotWarpLockPointsButtonClicked()));
    connect(resetPointsButton, SIGNAL(clicked()), this, SLOT(slotWarpResetPointsButtonClicked()));

    // Mode switch buttons
    connect(freeTransformButton, SIGNAL(clicked(bool)), this, SLOT(slotSetFreeTransformModeButtonClicked(bool)));
    connect(warpButton, SIGNAL(clicked(bool)), this, SLOT(slotSetWrapModeButtonClicked(bool)));

    // Connect all edit boxes to the Editing Finished signal
    connect(scaleXBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(scaleYBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(shearXBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(shearYBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(translateXBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(translateYBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(aXBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(aYBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(aZBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    connect(alphaBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(densityBox, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    // Connect Apply/Reset buttons
    connect(buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(slotButtonBoxClicked(QAbstractButton *)));
    setApplyResetDisabled(true);

    // Connect Decorations switcher
    Q_ASSERT(canvas);
    connect(showDecorationsBox, SIGNAL(toggled(bool)), canvas, SLOT(updateCanvas()));

    tooBigLabelWidget->hide();

    m_valuesChanged = false;*/
}

double KisToolTransformConfigWidget::radianToDegree(double rad)
{
    double piX2 = 2 * M_PI;

    if (rad < 0 || rad >= piX2) {
        rad = fmod(rad, piX2);
        if (rad < 0) {
            rad += piX2;
        }
    }

    return (rad * 360. / piX2);
}

void KisToolTransformConfigWidget::updateConfig(const ToolTransformArgs &config)
{
    if (config.mode() == ToolTransformArgs::FREE_TRANSFORM) {
        stackedWidget->setCurrentIndex(0);
        freeTransformButton->setChecked(true);
        warpButton->setChecked(false);
        scaleXBox->setValue(config.scaleX() * 100.);
        scaleYBox->setValue(config.scaleY() * 100.);
        shearXBox->setValue(config.shearX());
        shearYBox->setValue(config.shearY());
        translateXBox->setValue(config.translate().x());
        translateYBox->setValue(config.translate().y());
        aXBox->setValue(radianToDegree(config.aX()));
        aYBox->setValue(radianToDegree(config.aY()));
        aZBox->setValue(radianToDegree(config.aZ()));
    } else {
        stackedWidget->setCurrentIndex(1);
        freeTransformButton->setChecked(false);
        warpButton->setChecked(true);
        alphaBox->setValue(config.alpha());

        if (config.defaultPoints()) {
            densityBox->setValue(config.pointsPerLine());
        }


        cmbWarpType->setCurrentIndex((int)config.warpType());
        defaultRadioButton->setChecked(config.defaultPoints());
        customRadioButton->setChecked(!config.defaultPoints());
        defaultWarpWidget->setEnabled(config.defaultPoints());
        customWarpWidget->setEnabled(!config.defaultPoints());

        if (m_transaction->editWarpPoints()) {
            lockUnlockPointsButton->setText(i18n("Lock Points"));
        } else {
            lockUnlockPointsButton->setText(i18n("Unlock Points"));
        }
    }
}

void KisToolTransformConfigWidget::setApplyResetDisabled(bool disabled)
{
    QAbstractButton *applyButton = buttonBox->button(QDialogButtonBox::Apply);
    QAbstractButton *resetButton = buttonBox->button(QDialogButtonBox::Reset);

    Q_ASSERT(applyButton);
    Q_ASSERT(resetButton);

    applyButton->setDisabled(disabled);
    resetButton->setDisabled(disabled);
}

/*void KisToolTransformConfigWidget::slotEditingFinished()
{
    if (m_configChanged) {
        m_configChanged = false;
        emit sigConfigChanged(m_currentConfig);
    }

    setApplyResetDisabled(m_currentConfig.isIdentity(m_originalCenter));
}*/
