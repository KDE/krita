/*
 *  SPDX-FileCopyrightText: 2004, 2007, 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_auto_brush_widget.h"

#include "kis_image_config.h"
#include <math.h>
#include <kis_debug.h>
#include <QSpinBox>
#include <QToolButton>
#include <QImage>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

#include <kis_fixed_paint_device.h>
#include <kis_mask_generator.h>
#include <kis_slider_spin_box.h>
#include "kis_signals_blocker.h"
#include "kis_signal_compressor.h"
#include "kis_aspect_ratio_locker.h"
#include <KisAngleSelector.h>
#include <KisDoubleSpinBoxPluralHelper.h>
#include <KisWidgetConnectionUtils.h>
#include <kis_cubic_curve.h>
#include <kis_auto_brush_factory.h>
#include <KisGlobalResourcesInterface.h>
#include <KisAutoBrushModel.h>


struct KisAutoBrushWidget::Private {
    Private(KisAutoBrushModel *_model)
        : model(_model),
          previewCompressor(200, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    KisAutoBrushModel *model {0};
    KisSignalCompressor previewCompressor;
};



KisAutoBrushWidget::KisAutoBrushWidget(int maxBrushSize,
                                       KisAutoBrushModel *model,
                                       QWidget *parent, const char* name)
    : KisWdgAutoBrush(parent, name)
    , m_fadeAspectLocker(new KisAspectRatioLocker())
    , m_d(new Private(model))

{
    connectControl(comboBoxShape, m_d->model, "shape");
    connectControl(comboBoxMaskType, m_d->model, "type");

    inputRadius->setRange(0.01, maxBrushSize, 2);
    inputRadius->setExponentRatio(3.0);
    inputRadius->setSingleStep(1);
    inputRadius->setValue(5);
    inputRadius->setSuffix(i18n(" px"));
    inputRadius->setBlockUpdateSignalOnDrag(true);
    connectControl(inputRadius, m_d->model, "diameter");

    inputRatio->setRange(0.01, 1.0, 2);
    inputRatio->setSingleStep(0.01);
    inputRatio->setValue(1.0);
    inputRatio->setBlockUpdateSignalOnDrag(true);
    connectControl(inputRatio, m_d->model, "ratio");

    inputHFade->setRange(0.0, 1.0, 2);
    inputHFade->setSingleStep(0.01);
    inputHFade->setValue(0.5);

    inputVFade->setRange(0.0, 1.0, 2);
    inputVFade->setSingleStep(0.01);
    inputVFade->setValue(0.5);

    aspectButton->setKeepAspectRatio(true);

    m_fadeAspectLocker->connectSpinBoxes(inputHFade, inputVFade, aspectButton);
    m_fadeAspectLocker->setBlockUpdateSignalOnDrag(false);

    connect(m_fadeAspectLocker.data(), &KisAspectRatioLocker::sliderValueChanged,
            [this] () {
                m_d->model->sethorizontalFade(inputHFade->value());
                m_d->model->setverticalFade(inputVFade->value());
            });

    m_d->model->LAGER_QT(horizontalFade).bind([this] (qreal value) {
        KisSignalsBlocker b(inputHFade);
        inputHFade->setValue(value);
        m_fadeAspectLocker->updateAspect();
    });

    m_d->model->LAGER_QT(verticalFade).bind([this] (qreal value) {
        KisSignalsBlocker b(inputVFade);
        inputVFade->setValue(value);
        m_fadeAspectLocker->updateAspect();
    });

    inputSpikes->setRange(2, 20);
    inputSpikes->setValue(2);
    inputSpikes->setBlockUpdateSignalOnDrag(true);
    connectControl(inputSpikes, m_d->model, "spikes");

    inputRandomness->setRange(0, 100);
    inputRandomness->setValue(0);
    inputRandomness->setBlockUpdateSignalOnDrag(true);
    connectControl(inputRandomness, m_d->model, "randomness");

    inputAngle->setDecimals(0);
    connectControl(inputAngle, m_d->model, "angle");
    connectControl(spacingWidget, m_d->model, "aggregatedSpacing");

    density->setRange(0, 100, 0);
    density->setSingleStep(1);
    density->setValue(100);
    KisDoubleSpinBoxPluralHelper::install(density, [](double value) {
        return i18nc("{n} is the number value, % is the percent sign", "{n}%", value);
    });
    density->setBlockUpdateSignalOnDrag(true);
    connectControl(density, m_d->model, "density");

    connect(softnessCurve, &KisCurveWidget::modified, this, &KisAutoBrushWidget::slotCurveWidgetChanged);
    connect(m_d->model, &KisAutoBrushModel::curveStringChanged, this, &KisAutoBrushWidget::slotCurvePropertyChanged);
    m_d->model->LAGER_QT(curveString).nudge();

    QList<KoID> ids = KisMaskGenerator::maskGeneratorIds();
    for (int i = 0; i < ids.size(); i++) {
        comboBoxMaskType->insertItem(i, ids[i].name());
    }

    connect(m_d->model, &KisAutoBrushModel::typeChanged, this, &KisAutoBrushWidget::setStackedWidget);
    setStackedWidget(m_d->model->type());

    brushPreview->setIconSize(QSize(100, 100));

    connectControl(btnAntialiasing, m_d->model, "antialiasEdges");

    lager::watch(m_d->model->m_commonData, std::bind(&KisSignalCompressor::start, &m_d->previewCompressor));
    lager::watch(m_d->model->m_autoBrushData, std::bind(&KisSignalCompressor::start, &m_d->previewCompressor));
    lager::watch(m_d->model->m_commonBrushSizeData, std::bind(&KisSignalCompressor::start, &m_d->previewCompressor));

    connect(&m_d->previewCompressor, &KisSignalCompressor::timeout, this, &KisAutoBrushWidget::slotUpdateBrushPreview);

    slotUpdateBrushPreview();
}

KisAutoBrushWidget::~KisAutoBrushWidget()
{
}

void KisAutoBrushWidget::resizeEvent(QResizeEvent *)
{
    brushPreview->setMinimumHeight(brushPreview->width()); // dirty hack !
    brushPreview->setMaximumHeight(brushPreview->width()); // dirty hack !
}

void KisAutoBrushWidget::setStackedWidget(int index)
{
    if (index == 1) {
        stackedWidget->setCurrentIndex(1);
    }
    else {
        stackedWidget->setCurrentIndex(0);
    }
}

void KisAutoBrushWidget::slotCurveWidgetChanged()
{
    m_d->model->setcurveString(softnessCurve->curve().toString());
}

void KisAutoBrushWidget::slotCurvePropertyChanged(const QString &value)
{
    KisCubicCurve curve;

    if (!value.isEmpty()) {
        curve = KisCubicCurve(value);
    } else {
        curve.setPoint(0, QPointF(0.0, 1.0));
        curve.setPoint(1, QPointF(1.0, 0.0));
    }

    KisSignalsBlocker b(softnessCurve);
    softnessCurve->setCurve(curve);
}

void KisAutoBrushWidget::slotUpdateBrushPreview()
{
    KisAutoBrushFactory factory;

    QSharedPointer<KisAutoBrush> brush =
        factory.createBrush(*m_d->model->m_commonData,
                            m_d->model->bakedOptionData(),
                            KisGlobalResourcesInterface::instance())
            .resource<KisAutoBrush>();

    QImage pi(brush->image());

    double coeff = 1.0;
    int bPw = brushPreview->width() - 3;
    if (pi.width() > bPw) {
        coeff =  bPw / (double)pi.width();
    }
    int bPh = brushPreview->height() - 3;
    if (pi.height() > coeff * bPh) {
        coeff = bPh / (double)pi.height();
    }
    if (coeff < 1.0) {
        pi = pi.scaled((int)(coeff * pi.width()) , (int)(coeff * pi.height()),  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QPixmap p = QPixmap::fromImage(pi);
    brushPreview->setIcon(QIcon(p));
}

KisBrushSP KisAutoBrushWidget::brush()
{
    KisAutoBrushFactory factory;

    QSharedPointer<KisAutoBrush> brush =
        factory.createBrush(*m_d->model->m_commonData,
                            m_d->model->bakedOptionData(),
                            KisGlobalResourcesInterface::instance())
            .resource<KisAutoBrush>();

    return brush;
}
