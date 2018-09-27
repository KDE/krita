/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_lod_availability_widget.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QToolTip>


#include <kis_canvas_resource_provider.h>
#include <kis_slider_spin_box.h>
#include "kis_config.h"
#include <QWidgetAction>
#include <QMenu>

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
    Private() : chkLod(0), resourceManager(0) {}

    QCheckBox *chkLod;
    QPushButton *btnLod;
    QScopedPointer<QMenu> thresholdMenu;
    KisDoubleSliderSpinBox *thresholdSlider = 0;
    KoCanvasResourceManager *resourceManager;

    KisPaintopLodLimitations limitations;
    bool thresholdSupported = true;

    bool sizeThresholdPassed();
};

KisLodAvailabilityWidget::KisLodAvailabilityWidget(QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->chkLod = new QCheckBox(this);

    m_d->btnLod = new QPushButton(this);
    m_d->btnLod->setFlat(true);

    connect(m_d->btnLod, SIGNAL(clicked()), SLOT(showLodToolTip()));

    {
        m_d->thresholdMenu.reset(new QMenu());
        m_d->thresholdMenu->addSection(i18n("Enable after:"));

        m_d->btnLod->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_d->btnLod, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showLodThresholdWidget(QPoint)));

        KisConfig cfg(true);
        m_d->thresholdSlider = new KisDoubleSliderSpinBox(m_d->thresholdMenu.data());

        m_d->thresholdSlider->setRange(0, cfg.readEntry("maximumBrushSize", 1000), 2);
        m_d->thresholdSlider->setValue(100);
        m_d->thresholdSlider->setSingleStep(1);
        m_d->thresholdSlider->setExponentRatio(3.0);
        m_d->thresholdSlider->setSuffix(i18n(" px"));
        m_d->thresholdSlider->setBlockUpdateSignalOnDrag(true);

        QWidgetAction *sliderAction = new QWidgetAction(this);
        sliderAction->setDefaultWidget(m_d->thresholdSlider);

        m_d->thresholdMenu->addAction(sliderAction);
    }

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_d->chkLod);
    layout->addWidget(m_d->btnLod);

    layout->setSpacing(0);

    setLayout(layout);

    // set no limitations
    setLimitations(m_d->limitations);

    connect(m_d->chkLod, SIGNAL(toggled(bool)), SIGNAL(sigUserChangedLodAvailability(bool)));
    connect(m_d->thresholdSlider, SIGNAL(valueChanged(qreal)), SIGNAL(sigUserChangedLodThreshold(qreal)));
}

KisLodAvailabilityWidget::~KisLodAvailabilityWidget()
{
}

void KisLodAvailabilityWidget::showLodToolTip()
{
    QToolTip::showText(QCursor::pos(), m_d->btnLod->toolTip(), m_d->btnLod);
}

void KisLodAvailabilityWidget::showLodThresholdWidget(const QPoint &pos)
{
    Q_UNUSED(pos);

    if (m_d->thresholdSupported) {
        m_d->thresholdMenu->popup(QCursor::pos());
    }
}

void KisLodAvailabilityWidget::setLimitations(const KisPaintopLodLimitations &l)
{
    QString limitationsText;
    Q_FOREACH (const KoID &id, l.limitations) {
        limitationsText.append("<li>");
        limitationsText.append(id.name());
        limitationsText.append("</li>");
    }

    QString blockersText;
    Q_FOREACH (const KoID &id, l.blockers) {
        blockersText.append("<li>");
        blockersText.append(id.name());
        blockersText.append("</li>");
    }

    bool isBlocked = !l.blockers.isEmpty();
    bool isLimited = !l.limitations.isEmpty();

    m_d->thresholdSupported =
        m_d->resourceManager ?
        m_d->resourceManager->resource(KisCanvasResourceProvider::LodSizeThresholdSupported).toBool() :
        true;
    bool isBlockedByThreshold = !m_d->sizeThresholdPassed() && m_d->thresholdSupported;

    const QString text = !isBlocked && !isBlockedByThreshold && isLimited ?
        i18n("(Instant Preview)*") : i18n("Instant Preview");

    QString toolTip;

    if (isBlocked) {
        toolTip = i18nc("@info:tooltip",
                        "<p>Instant Preview Mode is "
                        "disabled by the following options:"
                        "<ul>%1</ul></p>", blockersText);
    } else if (isBlockedByThreshold) {
        const qreal lodThreshold = m_d->resourceManager->resource(KisCanvasResourceProvider::LodSizeThreshold).toDouble();
        const qreal size = m_d->resourceManager->resource(KisCanvasResourceProvider::Size).toDouble();

        toolTip = i18nc("@info:tooltip",
                        "<p>Instant Preview Mode is "
                        "disabled by instant preview threshold. "
                        "Please right-click here to change the threshold"
                        "<ul><li>Brush size %1</li>"
                        "<li>Threshold: %2</li></ul></p>",
                        size, lodThreshold);

    } else if (isLimited) {
        toolTip = i18nc("@info:tooltip",
                        "<p>Instant Preview may look different "
                        "from the final result. In case of troubles "
                        "try disabling the following options:"
                        "<ul>%1</ul></p>", limitationsText);
    } else {
        toolTip = i18nc("@info:tooltip", "<p>Instant Preview Mode is available</p>");
    }

    {
        QFont font;
        font.setStrikeOut(isBlocked || isBlockedByThreshold);
        m_d->chkLod->setEnabled(!isBlocked);
        m_d->btnLod->setEnabled(!isBlocked);
        m_d->btnLod->setFont(font);
        m_d->btnLod->setText(text);
        m_d->btnLod->setToolTip(toolTip);

        if (isBlocked) {
            /**
             * If LoD is really blocked by some limitation we sneakly reset
             * the checkbox to let the user know it is fully disabled.
             */

            KisSignalsBlocker b(m_d->chkLod);
            m_d->chkLod->setChecked(false);
        }
    }

    m_d->limitations = l;

    if (m_d->resourceManager) {
        const bool lodAvailableForUse =
            !isBlocked && !isBlockedByThreshold &&
            m_d->resourceManager->resource(KisCanvasResourceProvider::LodAvailability).toBool();

        m_d->resourceManager->setResource(KisCanvasResourceProvider::EffectiveLodAvailablility, lodAvailableForUse);
    }

}

void KisLodAvailabilityWidget::slotUserChangedLodAvailability(bool value)
{
    KisSignalsBlocker b(m_d->chkLod);

    m_d->chkLod->setChecked(value);
    setLimitations(m_d->limitations);
}

void KisLodAvailabilityWidget::slotUserChangedLodThreshold(qreal value)
{
    KisSignalsBlocker b(m_d->thresholdSlider);

    m_d->thresholdSlider->setValue(value);
    setLimitations(m_d->limitations);
}

void KisLodAvailabilityWidget::slotUserChangedSize(qreal value)
{
    Q_UNUSED(value);
    setLimitations(m_d->limitations);
}

void KisLodAvailabilityWidget::setCanvasResourceManager(KoCanvasResourceManager *resourceManager)
{
    m_d->resourceManager = resourceManager;
}

bool KisLodAvailabilityWidget::Private::sizeThresholdPassed()
{
    if (!resourceManager) return true;

    const qreal lodThreshold = resourceManager->resource(KisCanvasResourceProvider::LodSizeThreshold).toDouble();
    const qreal size = resourceManager->resource(KisCanvasResourceProvider::Size).toDouble();

    return size >= lodThreshold;
}
