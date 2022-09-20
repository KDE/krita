/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGShadeLineEditor.h"
#include "WGShadeSlider.h"

#include "ui_WdgWGShadeLineEditor.h"

#include <QCoreApplication>
#include <QResizeEvent>

WGShadeLineEditor::WGShadeLineEditor(QWidget *parent)
    : QFrame(parent, Qt::Popup)
    , m_model(new KisVisualColorModel)
    , m_ui(new Ui_WGShadeLineEditor)
    , m_previewLine(new WGShadeSlider(WGSelectorDisplayConfigSP(new WGSelectorDisplayConfig()), this, m_model))
    , m_iconSlider(new WGShadeSlider(WGSelectorDisplayConfigSP(new WGSelectorDisplayConfig()), this, m_model))
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    m_model->slotSetColorSpace(KoColorSpaceRegistry::instance()->rgb8());
    m_model->slotSetColor(KoColor(QColor(190, 50, 50), m_model->colorSpace()));

    m_ui->setupUi(this);

    m_previewLine->setObjectName(QString::fromUtf8("previewLine"));
    m_previewLine->setFixedHeight(24);

    m_ui->verticalLayout->addWidget(m_previewLine);

    m_ui->verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_previewLine->setModel(m_model);
    m_previewLine->slotSetChannelValues(m_model->channelValues());
    // since this widget is not shown by Qt, a resize event needs to be sent manually
    QResizeEvent event(QSize(128, 10), m_iconSlider->size());
    m_iconSlider->resize(128, 10);
    QCoreApplication::sendEvent(m_iconSlider, &event);
    m_iconSlider->hide();
    m_iconSlider->slotSetChannelValues(m_model->channelValues());
    connect(m_ui->sbRangeHue, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));
    connect(m_ui->sbRangeSaturation, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));
    connect(m_ui->sbRangeValue, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));
    connect(m_ui->sbOffsetHue, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));
    connect(m_ui->sbOffsetSaturation, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));
    connect(m_ui->sbOffsetValue, SIGNAL(valueChanged(double)), SLOT(slotValueChanged()));
    connect(m_ui->sbPatchCount, SIGNAL(valueChanged(int)), SLOT(slotPatchCountChanged(int)));
    connect(m_ui->rbSlider, SIGNAL(toggled(bool)), SLOT(slotSliderModeChanged(bool)));
    // disable spinBox when slider mode set
    connect(m_ui->rbColorPatches, SIGNAL(toggled(bool)), m_ui->sbPatchCount, SLOT(setEnabled(bool)));
}

WGShadeLineEditor::~WGShadeLineEditor()
{

}

WGConfig::ShadeLine WGShadeLineEditor::configuration() const
{
    WGConfig::ShadeLine cfg;
    cfg.gradient = QVector4D(m_ui->sbRangeHue->value(),
                             m_ui->sbRangeSaturation->value(),
                             m_ui->sbRangeValue->value(),
                             0);
    cfg.offset = QVector4D(m_ui->sbOffsetHue->value(),
                           m_ui->sbOffsetSaturation->value(),
                           m_ui->sbOffsetValue->value(),
                           0);
    cfg.patchCount = m_ui->rbSlider->isChecked() ? -1 : m_ui->sbPatchCount->value();
    return cfg;
}

void WGShadeLineEditor::setConfiguration(const WGConfig::ShadeLine &cfg, int lineIndex)
{
    m_ui->sbRangeHue->setValue(cfg.gradient.x());
    m_ui->sbRangeSaturation->setValue(cfg.gradient.y());
    m_ui->sbRangeValue->setValue(cfg.gradient.z());
    m_ui->sbOffsetHue->setValue(cfg.offset.x());
    m_ui->sbOffsetSaturation->setValue(cfg.offset.y());
    m_ui->sbOffsetValue->setValue(cfg.offset.z());
    if (cfg.patchCount > 0) {
        m_ui->rbColorPatches->setChecked(true);
        m_ui->sbPatchCount->setValue(cfg.patchCount);
    }
    else {
        m_ui->rbSlider->setChecked(true);
    }
    m_lineIndex = lineIndex;
}

QIcon WGShadeLineEditor::generateIcon(const WGConfig::ShadeLine &cfg)
{
    m_iconSlider->setGradient(cfg.gradient, cfg.offset);
    m_iconSlider->setDisplayMode(cfg.patchCount < 0, cfg.patchCount);
    return QIcon(QPixmap::fromImage(*m_iconSlider->background()));
}

void WGShadeLineEditor::hideEvent(QHideEvent *event)
{
    Q_EMIT sigEditorClosed(m_lineIndex);
    QWidget::hideEvent(event);
}

void WGShadeLineEditor::slotValueChanged()
{
    WGConfig::ShadeLine cfg = configuration();
    m_previewLine->setGradient(cfg.gradient, cfg.offset);
}

void WGShadeLineEditor::slotPatchCountChanged(int value)
{
    m_previewLine->setDisplayMode(false, value);
}

void WGShadeLineEditor::slotSliderModeChanged(bool enabled)
{
    m_previewLine->setDisplayMode(enabled, m_ui->sbPatchCount->value());
}
