/* This file is part of the KDE project
 * Copyright (C) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "colorsetchooser.h"

#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QPushButton>

#include <klocale.h>

#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerProvider.h>
#include <KoMainWindow.h>
#include <KoResource.h>
#include <KoColorSet.h>

#include "kis_pattern.h"
#include "kis_resource_server_provider.h"
#include "kis_workspace_resource.h"
#include "kis_view2.h"
#include <QGridLayout>
#include <klineedit.h>
#include <kis_canvas_resource_provider.h>

class ColorSetDelegate : public QAbstractItemDelegate
{
public:
    ColorSetDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    virtual ~ColorSetDelegate() {}
    /// reimplemented
    virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const {
        return option.decorationSize;
    }
};

void ColorSetDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    if (! index.isValid())
        return;

    KoResource* resource = static_cast<KoResource*>(index.internalPointer());
    KoColorSet* colorSet = static_cast<KoColorSet*>(resource);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }
    else {
        painter->setBrush(option.palette.text().color());
    }
    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, colorSet->name());

    int size = 7;
    for (int i = 0; i < colorSet->nColors() && i*size < option.rect.width(); i++) {
        QRect rect(option.rect.x() + i*size, option.rect.y() + option.rect.height() - size, size, size);
        painter->fillRect(rect, colorSet->getColor(i).color.toQColor());
    }
    
    painter->restore();
}

ColorSetChooser::ColorSetChooser(QWidget* parent): QWidget(parent)
{
    KoResourceServer<KoColorSet> * rserver = KoResourceServerProvider::instance()->paletteServer();
    KoAbstractResourceServerAdapter* adapter = new KoResourceServerAdapter<KoColorSet>(rserver);
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setItemDelegate(new ColorSetDelegate(this));
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->setRowHeight(30);
    m_itemChooser->setColumnCount(1);
    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SLOT(resourceSelected(KoResource*)));
    
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(m_itemChooser, 0, 0, 1, 2);
}

ColorSetChooser::~ColorSetChooser()
{
}

void ColorSetChooser::resourceSelected(KoResource* resource)
{
    emit paletteSelected(static_cast<KoColorSet*>(resource));
}
