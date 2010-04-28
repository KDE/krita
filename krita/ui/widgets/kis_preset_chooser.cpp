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

#include <QVBoxLayout>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <QStyleOptionViewItem>
#include <QSortFilterProxyModel>

#include <klocale.h>
#include <kstandarddirs.h>
#include <klineedit.h>

#include <KoResourceItemChooser.h>
#include <KoResourceModel.h>
#include <KoResourceServerAdapter.h>

#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_resource_server_provider.h"
#include "kis_global.h"
#include "kis_slider_spin_box.h"

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

    const QAbstractProxyModel* proxyModel = dynamic_cast<const QAbstractProxyModel*>(index.model());
    QModelIndex originalIndex = proxyModel->mapToSource(index);
    KisPaintOpPreset* preset = static_cast<KisPaintOpPreset*>(originalIndex.internalPointer());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        painter->fillRect(option.rect, option.palette.highlight());
    }

    painter->setPen(Qt::black);
    painter->drawText(option.rect.x() + 255, option.rect.y() + option.rect.height() - 5, preset->name());

    QRect previewRect = QRect(option.rect.x(), option.rect.y(), 250, option.rect.height());
    QImage preview = preset->image();

    if(preview.isNull()) {
        return;
    }
    painter->drawImage(previewRect.x(), previewRect.y(),
                       preview.scaled(previewRect.size(), Qt::KeepAspectRatio));
}

class KisPresetProxyModel : public QSortFilterProxyModel
{
public:
    KisPresetProxyModel(QObject * parent = 0)
        : QSortFilterProxyModel(parent), m_showAll(false){}
    virtual ~KisPresetProxyModel() {}

    ///Reimplemented
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if(m_paintopID.id().isEmpty())
            return true;

        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        if(!index.isValid())
            return false;

        KisPaintOpPreset* preset = static_cast<KisPaintOpPreset*>(index.internalPointer());
        return ((preset->paintOp() == m_paintopID || m_showAll) &&
                preset->name().contains(m_nameFilter, Qt::CaseInsensitive));
    }

    ///Set id for paintop to be accept by the proxy model, if not filter is set all
    ///presets will be shown.
    void setPresetFilter(const KoID &paintopID)
    {
        m_paintopID = paintopID;
    }

    /// Set a filter for preset name, only presets with name containing the string will be shown
    void setPresetNameFilter(const QString &nameFilter)
    {
        m_nameFilter = nameFilter;
    }

    void setShowAll(bool show)
    {
        m_showAll = show;
    }

private:
    KoID m_paintopID;
    QString m_nameFilter;
    bool m_showAll;
};

KisPresetChooser::KisPresetChooser(QWidget *parent, const char *name)
        : QWidget(parent)
{
    setObjectName(name);
    QVBoxLayout * layout = new QVBoxLayout(this);
    KoResourceServer<KisPaintOpPreset> * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    KoAbstractResourceServerAdapter* adapter = new KoResourceServerAdapter<KisPaintOpPreset>(rserver);
    m_chooser = new KoResourceItemChooser(adapter, this);
    m_chooser->showGetHotNewStuff(true, true);
    m_presetProxy = new KisPresetProxyModel(this);
    m_chooser->setProxyModel(m_presetProxy);
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

void KisPresetChooser::setPresetFilter(const KoID& paintopID)
{
    m_presetProxy->setPresetFilter(paintopID);
    m_presetProxy->invalidate();
}

void KisPresetChooser::searchTextChanged(const QString& searchString)
{
    m_presetProxy->setPresetNameFilter(searchString);
    m_presetProxy->invalidate();
}

void KisPresetChooser::setShowAll(bool show)
{
    m_presetProxy->setShowAll(show);
    m_presetProxy->invalidate();
}


#include "kis_preset_chooser.moc"

