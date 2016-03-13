/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_brush_hud.h"

#include <QVBoxLayout>
#include <QPointer>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QEvent>

#include "kis_uniform_paintop_property.h"
#include "kis_slider_based_paintop_property.h"
#include "kis_uniform_paintop_property_widget.h"
#include "kis_canvas_resource_provider.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"
#include "kis_signal_auto_connection.h"

#include "kis_debug.h"


struct KisBrushHud::Private
{
    QPointer<QLabel> lblPresetName;
    QPointer<QWidget> wdgProperties;
    QPointer<QScrollArea> wdgPropertiesArea;
    QPointer<QVBoxLayout> propertiesLayout;

    KisCanvasResourceProvider *provider;

    KisSignalAutoConnectionsStore connections;

    KisPaintOpPresetSP currentPreset;
};

KisBrushHud::KisBrushHud(KisCanvasResourceProvider *provider, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_d(new Private)
{
    m_d->provider = provider;

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_d->lblPresetName = new QLabel("<Preset Name>", this);
    layout->addWidget(m_d->lblPresetName);

    m_d->wdgPropertiesArea = new QScrollArea(this);
    m_d->wdgPropertiesArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_d->wdgPropertiesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_d->wdgPropertiesArea->setWidgetResizable(true);

    m_d->wdgProperties = new QWidget(this);
    m_d->propertiesLayout = new QVBoxLayout(this);
    m_d->propertiesLayout->setSpacing(0);
    m_d->propertiesLayout->setContentsMargins(0, 0, 22, 0);
    m_d->propertiesLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // not adding any widgets until explicitly requested

    m_d->wdgProperties->setLayout(m_d->propertiesLayout);
    m_d->wdgPropertiesArea->setWidget(m_d->wdgProperties);
    layout->addWidget(m_d->wdgPropertiesArea);

    setLayout(layout);
    setCursor(Qt::ArrowCursor);
}

KisBrushHud::~KisBrushHud()
{
}

void KisBrushHud::updateProperties()
{
    KisPaintOpPresetSP preset = m_d->provider->currentPreset();
    KisPaintOpSettingsSP settings = preset->settings();

    if (preset == m_d->currentPreset) return;
    m_d->currentPreset = preset;

    Q_FOREACH (QWidget *w, m_d->wdgProperties->findChildren<QWidget*>()) {
        w->deleteLater();
    }

    m_d->lblPresetName->setText(preset->name());

    QList<KisUniformPaintOpPropertySP> properties = settings->uniformProperties();
    Q_FOREACH(auto property, properties) {
        QWidget *w = 0;

        if (property->type() == KisUniformPaintOpProperty::Int) {
            w = new KisUniformPaintOpPropertyIntSlider(property, m_d->wdgProperties);
        } else if (property->type() == KisUniformPaintOpProperty::Double) {
            w = new KisUniformPaintOpPropertyDoubleSlider(property, m_d->wdgProperties);
        }

        if (w) {
            w->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            m_d->propertiesLayout->addWidget(w);
        }
    }
}

void KisBrushHud::showEvent(QShowEvent *event)
{
    m_d->connections.clear();
    m_d->connections.addUniqueConnection(
        m_d->provider->resourceManager(), SIGNAL(canvasResourceChanged(int, QVariant)),
        this, SLOT(slotCanvasResourceChanged(int, QVariant)));

    QWidget::showEvent(event);
}

void KisBrushHud::hideEvent(QHideEvent *event)
{
    m_d->connections.clear();
    QWidget::hideEvent(event);
}

void KisBrushHud::slotCanvasResourceChanged(int key, const QVariant &resource)
{
    if (key != KisCanvasResourceProvider::CurrentPaintOpPreset) return;
    updateProperties();
}

void KisBrushHud::paintEvent(QPaintEvent *event)
{
    QColor bgColor = palette().color(QPalette::Window);

    QPainter painter(this);
    painter.fillRect(rect() & event->rect(), bgColor);
    painter.end();

    QWidget::paintEvent(event);
}

bool KisBrushHud::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TabletPress:
    case QEvent::TabletMove:
    case QEvent::TabletRelease:
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
        event->accept();
        return true;
    }

    return QWidget::event(event);
}
