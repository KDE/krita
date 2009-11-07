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
    m_model = new MyBrushResourcesListModel(m_options->lstBrushes);
    m_options->lstBrushes->setModel(m_model);
    m_options->lstBrushes->setItemDelegate(new MyPaintBrushResourceDelegate(m_options->lstBrushes));
    m_options->lstBrushes->setCurrentIndex(m_model->index(0));
    connect(m_options->lstBrushes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(brushSelected(const QModelIndex&)));
    connect(m_options->lstBrushes, SIGNAL(activated(const QModelIndex&)), this, SLOT(brushSelected(const QModelIndex&)));
}


MyPaintSettingsWidget::~ MyPaintSettingsWidget()
{
}

void  MyPaintSettingsWidget::setConfiguration( const KisPropertiesConfiguration * config)
{
    const_cast<KisPropertiesConfiguration*>(config)->dump();
    // XXX: set the active brush
    Q_UNUSED(config);
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
    config->dump();
    config->setProperty( "paintop", "mypaintbrush"); // XXX: make this a const id string
    config->setProperty("filename", m_activeBrushFilename);
    // XXX: set current settings of the brush, not just the filename?
    config->dump();
}


void MyPaintSettingsWidget::brushSelected(const QModelIndex& index)
{

    m_activeBrushFilename = m_model->data(index).toString();
    QFileInfo info(m_activeBrushFilename);
    m_options->lblBrushName->setText(info.baseName());
}

MyPaintBrushResource* MyPaintSettingsWidget::brush() const
{
    return m_model->brush(m_activeBrushFilename);
}
