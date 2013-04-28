/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "mypaint_paintop_settings_widget.h"

#include <math.h>
#include <QtGui>
#include <QStyledItemDelegate>
#include <QVariant>
#include <QAbstractListModel>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_debug.h>
#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include "mypaint_paintop_factory.h"
#include "mypaint_paintop_settings.h"
#include "mypaint_brush_resource.h"
#include "mybrush_resources_listmodel.h"

class MyPaintBrushResourceDelegate : public QStyledItemDelegate
{
public:

    MyPaintBrushResourceDelegate(QListView* parent)
        : QStyledItemDelegate(parent)
        , m_parent(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {

        QPixmap pixmap = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
        QRect rect = option.rect;

        if (!(option.state & QStyle::State_Enabled))
            painter->setOpacity(0.4);

        painter->fillRect(rect, Qt::white);
        painter->setPen(QPen(QBrush(Qt::lightGray), 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rect.adjusted(1, 1, -1, -1));
        painter->drawPixmap(rect.center() - pixmap.rect().center(), pixmap);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const {

        Q_UNUSED(option);
        Q_UNUSED(index);
         return m_parent->iconSize();
    }

private:

    QListView* m_parent;
};


MyPaintSettingsWidget:: MyPaintSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_options = new Ui::WdgMyPaintOptions();
    m_options->setupUi(this);

    m_options->radiusSlider->setRange(-2.0, 5.0, 2);
    m_options->radiusSlider->setValue(2.0);
}


MyPaintSettingsWidget::~ MyPaintSettingsWidget()
{
}

void  MyPaintSettingsWidget::setConfiguration( const KisPropertiesConfiguration * config)
{
    m_activeBrushFilename = config->getString("filename");

    MyPaintBrushResource* brush;
    MyPaintFactory *factory = static_cast<MyPaintFactory*>(KisPaintOpRegistry::instance()->get("mypaintbrush"));
    brush = factory->brush(m_activeBrushFilename);
    m_options->radiusSlider->setValue(brush->setting_value_by_cname("radius_logarithmic"));

    connect(m_options->radiusSlider, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationUpdated()));
}

KisPropertiesConfiguration*  MyPaintSettingsWidget::configuration() const
{
    MyPaintSettings* settings = new MyPaintSettings();
    settings->setOptionsWidget(const_cast<MyPaintSettingsWidget*>(this));
    writeConfiguration(settings);

    settings->dump();
    return settings;
}

void MyPaintSettingsWidget::writeConfiguration( KisPropertiesConfiguration* config ) const
{
    config->setProperty( "paintop", "mypaintbrush"); // XXX: make this a const id string
    config->setProperty("filename", m_activeBrushFilename);
    config->setProperty("radius_logarithmic", m_options->radiusSlider->value());
    // XXX: set current settings of the brush, not just the filename?
    config->dump();
}

MyPaintBrushResource* MyPaintSettingsWidget::brush() const
{
    return m_model->brush(m_activeBrushFilename);
}

void MyPaintSettingsWidget::changePaintOpSize(qreal x, qreal /*y*/)
{
    float value = expf(m_options->radiusSlider->value()) + x;
    m_options->radiusSlider->setValue(log(value));
}

QSizeF MyPaintSettingsWidget::paintOpSize() const
{
    // TODO: return size of the brush
    return KisPaintOpSettingsWidget::paintOpSize();
}
