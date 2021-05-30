/*
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QToolButton>
#include <QCheckBox>
#include <QHBoxLayout>

#include <kis_slider_spin_box.h>
#include <kis_color_button.h>
#include <kis_icon_utils.h>

#include "KisGradientColorEditor.h"

class Q_DECL_HIDDEN KisGradientColorEditor::Private
{
public:
    KisDoubleSliderSpinBox *positionSlider{nullptr};
    QToolButton *colorTypeForegroundButton{nullptr};
    QToolButton *colorTypeBackgroundButton{nullptr};
    QToolButton *colorTypeCustomButton{nullptr};
    QCheckBox *transparentCheckBox{nullptr};
    KisColorButton *colorButton{nullptr};
    KisDoubleSliderSpinBox *opacitySlider{nullptr};
};

KisGradientColorEditor::KisGradientColorEditor(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    m_d->colorTypeForegroundButton = new QToolButton;
    m_d->colorTypeForegroundButton->setCheckable(true);
    m_d->colorTypeForegroundButton->setChecked(true);
    m_d->colorTypeForegroundButton->setAutoExclusive(true);
    m_d->colorTypeForegroundButton->setAutoRaise(true);
    m_d->colorTypeForegroundButton->setIcon(KisIconUtils::loadIcon("object-order-lower-calligra"));
    m_d->colorTypeForegroundButton->setToolTip(i18nc("Button to change the gradient stop type to foreground", "Foreground color"));
    connect(m_d->colorTypeForegroundButton, &QToolButton::toggled,
        [this](bool toggled)
        {
            if (toggled) {
                setColorType(KisGradientWidgetsUtils::Foreground);
            }
            emit colorTypeChanged(KisGradientWidgetsUtils::Foreground);
        });

    m_d->colorTypeBackgroundButton = new QToolButton;
    m_d->colorTypeBackgroundButton->setCheckable(true);
    m_d->colorTypeBackgroundButton->setAutoExclusive(true);
    m_d->colorTypeBackgroundButton->setAutoRaise(true);
    m_d->colorTypeBackgroundButton->setIcon(KisIconUtils::loadIcon("object-order-raise-calligra"));
    m_d->colorTypeBackgroundButton->setToolTip(i18nc("Button to change the gradient stop type to background", "Background color"));
    connect(m_d->colorTypeBackgroundButton, &QToolButton::toggled,
        [this](bool toggled)
        {
            if (toggled) {
                setColorType(KisGradientWidgetsUtils::Background);
            }
            emit colorTypeChanged(KisGradientWidgetsUtils::Background);
        });

    m_d->colorTypeCustomButton = new QToolButton;
    m_d->colorTypeCustomButton->setCheckable(true);
    m_d->colorTypeCustomButton->setAutoExclusive(true);
    m_d->colorTypeCustomButton->setAutoRaise(true);
    m_d->colorTypeCustomButton->setIcon(KisIconUtils::loadIcon("wheel-sectors"));
    m_d->colorTypeCustomButton->setToolTip(i18nc("Button to change the gradient stop type to custom color", "Custom color"));
    connect(m_d->colorTypeCustomButton, &QToolButton::toggled,
        [this](bool toggled)
        {
            if (toggled) {
                setColorType(KisGradientWidgetsUtils::Custom);
            }
            emit colorTypeChanged(KisGradientWidgetsUtils::Custom);
        });

    QWidget *colorTypeButtonsContainer = new QWidget;

    m_d->transparentCheckBox = new QCheckBox;
    m_d->transparentCheckBox->setText(i18n("Transparent"));
    m_d->transparentCheckBox->setProperty("isBeingUsed", true);
    connect(m_d->transparentCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(transparentToggled(bool)));

    m_d->colorButton = new KisColorButton;
    m_d->colorButton->setVisible(false);
    connect(m_d->colorButton, SIGNAL(changed(KoColor)), this, SIGNAL(colorChanged(KoColor)));

    m_d->opacitySlider = new KisDoubleSliderSpinBox;
    m_d->opacitySlider->setRange(0, 100, 2);
    m_d->opacitySlider->setPrefix(i18n("Opacity: "));
    m_d->opacitySlider->setSuffix(i18n("%"));
    m_d->opacitySlider->setVisible(false);
    connect(m_d->opacitySlider, SIGNAL(valueChanged(double)), this, SIGNAL(opacityChanged(qreal)));

    m_d->positionSlider = new KisDoubleSliderSpinBox;
    m_d->positionSlider->setRange(0, 100, 2);
    m_d->positionSlider->setPrefix(i18n("Position: "));
    m_d->positionSlider->setSuffix(i18n("%"));
    connect(m_d->positionSlider, SIGNAL(valueChanged(double)), this, SIGNAL(positionChanged(qreal)));

    QHBoxLayout *colorTypeButtonsLayout = new QHBoxLayout;
    colorTypeButtonsLayout->setMargin(0);
    colorTypeButtonsLayout->setSpacing(0);
    colorTypeButtonsLayout->addWidget(m_d->colorTypeForegroundButton);
    colorTypeButtonsLayout->addWidget(m_d->colorTypeBackgroundButton);
    colorTypeButtonsLayout->addWidget(m_d->colorTypeCustomButton);
    colorTypeButtonsContainer->setLayout(colorTypeButtonsLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(5);
    mainLayout->addWidget(colorTypeButtonsContainer);
    mainLayout->addWidget(m_d->transparentCheckBox);
    mainLayout->addWidget(m_d->colorButton);
    mainLayout->addWidget(m_d->opacitySlider);
    mainLayout->addStretch();
    mainLayout->addWidget(m_d->positionSlider);
    setLayout(mainLayout);
}

KisGradientColorEditor::KisGradientColorEditor(const KisGradientColorEditor &other)
    : QWidget()
{
    setPosition(other.position());
    setColorType(other.colorType());
    setTransparent(other.transparent());
    setColor(other.color());
    setOpacity(other.opacity());
}

KisGradientColorEditor::~KisGradientColorEditor()
{}

qreal KisGradientColorEditor::position() const
{
    return m_d->positionSlider->value();
}

KisGradientWidgetsUtils::ColorType KisGradientColorEditor::colorType() const
{
    if (m_d->colorTypeForegroundButton->isChecked()) {
        return KisGradientWidgetsUtils::Foreground;
    } else if (m_d->colorTypeBackgroundButton->isChecked()) {
        return KisGradientWidgetsUtils::Background;
    } else {
        return KisGradientWidgetsUtils::Custom;
    }
}

KoColor KisGradientColorEditor::color() const
{
    return m_d->colorButton->color();
}

bool KisGradientColorEditor::transparent() const
{
    return m_d->transparentCheckBox->isChecked();
}

qreal KisGradientColorEditor::opacity() const
{
    return m_d->opacitySlider->value();
}

void KisGradientColorEditor::setPosition(qreal position)
{
    m_d->positionSlider->setValue(position);
}

void KisGradientColorEditor::setColorType(KisGradientWidgetsUtils::ColorType type)
{
    if (type == KisGradientWidgetsUtils::Foreground) {
        m_d->colorTypeForegroundButton->setChecked(true);
    } else if (type == KisGradientWidgetsUtils::Background) {
        m_d->colorTypeBackgroundButton->setChecked(true);
    } else {
        m_d->colorTypeCustomButton->setChecked(true);
    }

    // "if" to avoid flickering. setUpdatesEnabled doesn't seems to work here?
    if (type == KisGradientWidgetsUtils::Custom) {
        m_d->transparentCheckBox->setVisible(false);
        m_d->colorButton->setVisible(true);
        m_d->opacitySlider->setVisible(true);
    } else {
        m_d->colorButton->setVisible(false);
        m_d->opacitySlider->setVisible(false);
        m_d->transparentCheckBox->setVisible(m_d->transparentCheckBox->property("isBeingUsed").toBool());
    }

    if (type != colorType()) {
        emit colorTypeChanged(type);
    }
}

void KisGradientColorEditor::setTransparent(bool checked)
{
    m_d->transparentCheckBox->setChecked(checked);
}

void KisGradientColorEditor::setColor(KoColor color)
{
    color.setOpacity(1.0);
    if (color == m_d->colorButton->color()) {
        return;
    }
    m_d->colorButton->setColor(color);
}

void KisGradientColorEditor::setOpacity(qreal opacity)
{
    m_d->opacitySlider->setValue(opacity);
}

void KisGradientColorEditor::setUseTransParentCheckBox(bool use)
{
    m_d->transparentCheckBox->setProperty("isBeingUsed", use);
    if (colorType() != KisGradientWidgetsUtils::Custom) {
        m_d->transparentCheckBox->setVisible(use);
    }
}

void KisGradientColorEditor::setUsePositionSlider(bool use)
{
    m_d->positionSlider->setVisible(use);
}

void KisGradientColorEditor::setPositionSliderEnabled(bool enabled)
{
    m_d->positionSlider->setEnabled(enabled);
}
