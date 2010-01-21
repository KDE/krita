/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_preset_chooser.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <QStyleOptionViewItem>

#include <klocale.h>
#include <kstandarddirs.h>
#include <KoResourceItemChooser.h>
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_resource_server_provider.h"
#include <KoResourceServerAdapter.h>

#include "kis_global.h"

/// The resource item delegate for rendering the resource preview
class KisPresetDelegate : public QAbstractItemDelegate
{
public:
    KisPresetDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    virtual ~KisPresetDelegate() {}
    /// reimplemented
    virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const {
        return option.decorationSize;
    }
};

void KisPresetDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (! index.isValid())
        return;

    KisPaintOpPreset* preset = static_cast<KisPaintOpPreset*>(index.internalPointer());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        painter->fillRect(option.rect, option.palette.highlight());
    }

    painter->setPen(Qt::black);
    painter->drawText(option.rect.x() + 5, option.rect.y() + option.rect.height() - 5, preset->name());
    
    QRect previewRect = option.rect.adjusted(200, 0, -20, 0);
    QImage preview = preset->settings()->sampleStroke(previewRect.size());
    painter->drawImage(previewRect.x(), previewRect.y(),
                       preview.scaled(previewRect.size(), Qt::KeepAspectRatio));
}

KisPresetChooser::KisPresetChooser(QWidget *parent, const char *name)
        : QWidget(parent)
{
    setObjectName(name);
    QHBoxLayout * layout = new QHBoxLayout(this);
    KoResourceServer<KisPaintOpPreset> * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    KoAbstractResourceServerAdapter* adapter = new KoResourceServerAdapter<KisPaintOpPreset>(rserver);
    m_chooser = new KoResourceItemChooser(adapter, this);
    m_chooser->setColumnCount(1);
    m_chooser->setRowHeight(60);
    m_chooser->setItemDelegate(new KisPresetDelegate(this));
    layout->addWidget(m_chooser);
    
    connect(m_chooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(resourceSelected(KoResource*)));
}

KisPresetChooser::~KisPresetChooser()
{
}



#include "kis_preset_chooser.moc"

