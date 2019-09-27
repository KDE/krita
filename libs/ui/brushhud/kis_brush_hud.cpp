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

#include <QGuiApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPointer>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QEvent>
#include <QToolButton>


#include "kis_uniform_paintop_property.h"
#include "kis_slider_based_paintop_property.h"
#include "kis_uniform_paintop_property_widget.h"
#include "kis_canvas_resource_provider.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"
#include "kis_signal_auto_connection.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_icon_utils.h"
#include "kis_dlg_brush_hud_config.h"
#include "kis_brush_hud_properties_config.h"
#include "kis_elided_label.h"

#include "kis_debug.h"


struct KisBrushHud::Private
{
    QPointer<KisElidedLabel> lblPresetName;
    QPointer<QLabel> lblPresetIcon;
    QPointer<QWidget> wdgProperties;
    QPointer<QScrollArea> wdgPropertiesArea;
    QPointer<QVBoxLayout> propertiesLayout;
    QPointer<QToolButton> btnConfigure;

    KisCanvasResourceProvider *provider;

    KisSignalAutoConnectionsStore connections;
    KisSignalAutoConnectionsStore presetConnections;

    KisPaintOpPresetSP currentPreset;
};

KisBrushHud::KisBrushHud(KisCanvasResourceProvider *provider, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_d(new Private)
{
    m_d->provider = provider;

    QVBoxLayout *layout = new QVBoxLayout();

    QHBoxLayout *labelLayout = new QHBoxLayout();
    m_d->lblPresetIcon = new QLabel(this);
    const QSize iconSize = QSize(22,22);
    m_d->lblPresetIcon->setMinimumSize(iconSize);
    m_d->lblPresetIcon->setMaximumSize(iconSize);
    m_d->lblPresetIcon->setScaledContents(true);

    m_d->lblPresetName = new KisElidedLabel("<Preset Name>", Qt::ElideMiddle, this);

    m_d->btnConfigure = new QToolButton(this);
    m_d->btnConfigure->setAutoRaise(true);



    connect(m_d->btnConfigure, SIGNAL(clicked()), SLOT(slotConfigBrushHud()));

    labelLayout->addWidget(m_d->lblPresetIcon);
    labelLayout->addWidget(m_d->lblPresetName);
    labelLayout->addWidget(m_d->btnConfigure);

    layout->addLayout(labelLayout);

    m_d->wdgPropertiesArea = new QScrollArea(this);
    m_d->wdgPropertiesArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_d->wdgPropertiesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_d->wdgPropertiesArea->setWidgetResizable(true);

    m_d->wdgProperties = new QWidget(this);
    m_d->propertiesLayout = new QVBoxLayout(this);
    m_d->propertiesLayout->setSpacing(0);
    m_d->propertiesLayout->setContentsMargins(0, 0, 22, 0);
    m_d->propertiesLayout->setSizeConstraint(QLayout::SetMinimumSize);

    // not adding any widgets until explicitly requested

    m_d->wdgProperties->setLayout(m_d->propertiesLayout);
    m_d->wdgPropertiesArea->setWidget(m_d->wdgProperties);
    layout->addWidget(m_d->wdgPropertiesArea);

    updateIcons();

    setLayout(layout);
    setCursor(Qt::ArrowCursor);

    // Prevent tablet events from being captured by the canvas
    setAttribute(Qt::WA_NoMousePropagation, true);
}



KisBrushHud::~KisBrushHud()
{
}

QSize KisBrushHud::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    return QSize(size.width(), parentWidget()->height());
}

void KisBrushHud::updateIcons()
{
    this->setPalette(qApp->palette());
    for(int i=0; i<this->children().size(); i++) {
        QWidget *w = qobject_cast<QWidget*>(this->children().at(i));
        if (w) {
            w->setPalette(qApp->palette());
        }
    }
    for(int i=0; i<m_d->wdgProperties->children().size(); i++) {
        KisUniformPaintOpPropertyWidget *w = qobject_cast<KisUniformPaintOpPropertyWidget*>(m_d->wdgProperties->children().at(i));
        if (w) {
            w->slotThemeChanged(qApp->palette());
        }
    }
    m_d->btnConfigure->setIcon(KisIconUtils::loadIcon("applications-system"));
}

void KisBrushHud::slotReloadProperties()
{
    m_d->presetConnections.clear();
    clearProperties();
    updateProperties();
}

void KisBrushHud::clearProperties() const
{
    while (m_d->propertiesLayout->count()) {
        QLayoutItem *item = m_d->propertiesLayout->takeAt(0);

        QWidget *w = item->widget();
        if (w) {
            w->deleteLater();
        }

        delete item;
    }

    m_d->currentPreset.clear();
}

void KisBrushHud::updateProperties()
{
    KisPaintOpPresetSP preset = m_d->provider->currentPreset();

    if (preset == m_d->currentPreset) return;

    m_d->presetConnections.clear();
    clearProperties();

    m_d->currentPreset = preset;
    m_d->presetConnections.addConnection(
        m_d->currentPreset->updateProxy(), SIGNAL(sigUniformPropertiesChanged()),
        this, SLOT(slotReloadProperties()));

    m_d->lblPresetIcon->setPixmap(QPixmap::fromImage(preset->image()));
    m_d->lblPresetName->setLongText(preset->name());

    QList<KisUniformPaintOpPropertySP> properties;

    {
        QList<KisUniformPaintOpPropertySP> allProperties = preset->uniformProperties();
        QList<KisUniformPaintOpPropertySP> discardedProperties;

        KisBrushHudPropertiesConfig cfg;
        cfg.filterProperties(preset->paintOp().id(),
                             allProperties,
                             &properties,
                             &discardedProperties);
    }

    Q_FOREACH(auto property, properties) {
        QWidget *w = 0;

        if (!property->isVisible()) continue;

        if (property->type() == KisUniformPaintOpProperty::Int) {
            w = new KisUniformPaintOpPropertyIntSlider(property, m_d->wdgProperties);
        } else if (property->type() == KisUniformPaintOpProperty::Double) {
            w = new KisUniformPaintOpPropertyDoubleSlider(property, m_d->wdgProperties);
        } else if (property->type() == KisUniformPaintOpProperty::Bool) {
            w = new KisUniformPaintOpPropertyCheckBox(property, m_d->wdgProperties);
        } else if (property->type() == KisUniformPaintOpProperty::Combo) {
            w = new KisUniformPaintOpPropertyComboBox(property, m_d->wdgProperties);
        }

        if (w) {
            w->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            m_d->propertiesLayout->addWidget(w);
        }
    }

    m_d->propertiesLayout->addStretch();
    resize(sizeHint());
}

void KisBrushHud::showEvent(QShowEvent *event)
{
    m_d->connections.clear();
    m_d->connections.addUniqueConnection(
        m_d->provider->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
        this, SLOT(slotCanvasResourceChanged(int,QVariant)));

    updateProperties();

    QWidget::showEvent(event);
}

void KisBrushHud::hideEvent(QHideEvent *event)
{
    m_d->connections.clear();
    QWidget::hideEvent(event);

    clearProperties();
}

void KisBrushHud::slotCanvasResourceChanged(int key, const QVariant &resource)
{
    Q_UNUSED(resource);

    if (key == KisCanvasResourceProvider::CurrentPaintOpPreset) {
        updateProperties();
    }
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
        // Allow the tablet event to be translated to a mouse event on certain platforms
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
        event->accept();
        return true;
    default:
        break;
    }

    return QWidget::event(event);
}

void KisBrushHud::slotConfigBrushHud()
{
    if (!m_d->currentPreset) return;

    KisDlgConfigureBrushHud dlg(m_d->currentPreset);
    dlg.exec();

    slotReloadProperties();
}
