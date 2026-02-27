/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_lod_availability_widget.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QToolTip>


#include <kis_canvas_resource_provider.h>
#include <kis_slider_spin_box.h>
#include "kis_image_config.h"
#include <QWidgetAction>
#include <QMenu>
#include <KisLongPressEventFilter.h>
#include <KisWidgetConnectionUtils.h>

#include "kis_signals_blocker.h"


namespace {
/**
 * These strings are added intentionally so we could relayout the threshold slider after
 * the string freeze for 4.0. Please translate them :)
 */
static const KLocalizedString stringForInstantPreviewThreshold1 = ki18nc("@label:slider", "Threshold:");
static const KLocalizedString stringForInstantPreviewThreshold2 = ki18nc("@label:slider", "Instant preview threshold:");
}

struct KisLodAvailabilityWidget::Private
{
    QCheckBox *chkLod {nullptr};
    QPushButton *btnLod {nullptr};
    QScopedPointer<QMenu> thresholdMenu;
    KisDoubleSliderSpinBox *thresholdSlider {nullptr};

    QScopedPointer<KisLodAvailabilityModel> model;
};

KisLodAvailabilityWidget::KisLodAvailabilityWidget(QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->chkLod = new QCheckBox(this);

    m_d->btnLod = new QPushButton(this);
    m_d->btnLod->setFlat(true);
    m_d->btnLod->setProperty(KisLongPressEventFilter::ENABLED_PROPERTY, true);

    connect(m_d->btnLod, SIGNAL(clicked()), SLOT(showLodToolTip()));

    {
        m_d->thresholdMenu.reset(new QMenu());
        m_d->thresholdMenu->addSection(i18n("Enable after:"));

        m_d->btnLod->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_d->btnLod, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showLodThresholdWidget(QPoint)));

        KisImageConfig cfg(true);
        m_d->thresholdSlider = new KisDoubleSliderSpinBox(m_d->thresholdMenu.data());

        m_d->thresholdSlider->setRange(0, cfg.maxBrushSize(), 2);
        m_d->thresholdSlider->setValue(100);
        m_d->thresholdSlider->setSingleStep(1);
        m_d->thresholdSlider->setExponentRatio(3.0);
        m_d->thresholdSlider->setSuffix(i18n(" px"));
        m_d->thresholdSlider->setBlockUpdateSignalOnDrag(true);

        QWidgetAction *sliderAction = new QWidgetAction(this);
        sliderAction->setDefaultWidget(m_d->thresholdSlider);

        m_d->thresholdMenu->addAction(sliderAction);
    }

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(m_d->chkLod);
    layout->addWidget(m_d->btnLod);
}

KisLodAvailabilityWidget::~KisLodAvailabilityWidget()
{
}

void KisLodAvailabilityWidget::setLodAvailabilityModel(KisLodAvailabilityModel *model)
{
    m_d->model.reset(model);

    m_d->model->LAGER_QT(availabilityState).bind(
        kismpl::unzip_wrapper(std::bind(&KisLodAvailabilityWidget::slotLodAvailabilityStateChanged,
                                        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

    connect(m_d->chkLod, &QCheckBox::toggled,
            m_d->model.data(), &KisLodAvailabilityModel::setisLodUserAllowed);

    using namespace KisWidgetConnectionUtils;
    connectControl(m_d->thresholdSlider, m_d->model.data(), "lodSizeThreshold");
}

void KisLodAvailabilityWidget::showLodToolTip()
{
    QToolTip::showText(QCursor::pos(), m_d->btnLod->toolTip(), m_d->btnLod);
}

void KisLodAvailabilityWidget::showLodThresholdWidget(const QPoint &pos)
{
    Q_UNUSED(pos);

    if (m_d->model && m_d->model->isLodSizeThresholdSupported()) {
        m_d->thresholdMenu->popup(QCursor::pos());
    }
}

void KisLodAvailabilityWidget::slotLodAvailabilityStateChanged(KisLodAvailabilityModel::AvailabilityState state, const KisPaintopLodLimitations &l, bool isLodUserAllowed)
{
    QString toolTip;

    if (state == KisLodAvailabilityModel::BlockedFully) {
        QString blockersText;
        Q_FOREACH (const KoID &id, l.blockers) {
            blockersText.append("<li>");
            blockersText.append(id.name());
            blockersText.append("</li>");
        }

        toolTip = i18nc("@info:tooltip",
                        "<p>Instant Preview Mode is "
                        "disabled by the following options:"
                        "<ul>%1</ul></p>", blockersText);
    } else if (state == KisLodAvailabilityModel::BlockedByThreshold) {

        const qreal effectiveBrushSize = m_d->model->effectiveBrushSize.get();
        const qreal sizeThreshold = m_d->model->lodSizeThreshold();

        toolTip = i18nc("@info:tooltip",
                        "<p>Instant Preview Mode is "
                        "disabled by instant preview threshold. "
                        "Please right-click here to change the threshold"
                        "<ul><li>Brush size %1</li>"
                        "<li>Threshold: %2</li></ul></p>",
                        effectiveBrushSize, sizeThreshold);

    } else if (state == KisLodAvailabilityModel::Limited) {

        QString limitationsText;
        Q_FOREACH (const KoID &id, l.limitations) {
            limitationsText.append("<li>");
            limitationsText.append(id.name());
            limitationsText.append("</li>");
        }

        toolTip = i18nc("@info:tooltip",
                        "<p>Instant Preview may look different "
                        "from the final result. In case of troubles "
                        "try disabling the following options:"
                        "<ul>%1</ul></p>", limitationsText);
    } else {
        toolTip = i18nc("@info:tooltip", "<p>Instant Preview Mode is available</p>");
    }

    const QString text = state == KisLodAvailabilityModel::Limited ?
        i18n("(Instant Preview)*") : i18n("Instant Preview");


    {
        QFont font;
        font.setStrikeOut(state >= KisLodAvailabilityModel::BlockedByThreshold);
        m_d->chkLod->setEnabled(state < KisLodAvailabilityModel::BlockedFully);
        m_d->btnLod->setEnabled(state < KisLodAvailabilityModel::BlockedFully);
        m_d->btnLod->setFont(font);
        m_d->btnLod->setText(text);
        m_d->btnLod->setToolTip(toolTip);

        if (state == KisLodAvailabilityModel::BlockedFully) {
            /**
             * If LoD is really blocked by some limitation we sneakily reset
             * the checkbox to let the user know it is fully disabled.
             */

            KisSignalsBlocker b(m_d->chkLod);
            m_d->chkLod->setChecked(false);
        } else {
            KisSignalsBlocker b(m_d->chkLod);
            m_d->chkLod->setChecked(isLodUserAllowed);
        }
    }
}
