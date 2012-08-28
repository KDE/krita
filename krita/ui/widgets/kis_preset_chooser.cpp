/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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
#include <QApplication>

#include <klocale.h>
#include <kstandarddirs.h>
#include <klineedit.h>

#include <KoIcon.h>
#include <KoResourceItemChooser.h>
#include <KoResourceModel.h>
#include <KoResourceServerAdapter.h>

#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_resource_server_provider.h"
#include "kis_global.h"
#include "kis_slider_spin_box.h"
#include "kis_config.h"

/// The resource item delegate for rendering the resource preview
class KisPresetDelegate : public QAbstractItemDelegate
{
public:
    KisPresetDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent), m_showText(false) {}
    virtual ~KisPresetDelegate() {}
    /// reimplemented
    virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const {
        return option.decorationSize;
    }

    void setShowText(bool showText) {
        m_showText = showText;
    }

private:
    bool m_showText;
};

void KisPresetDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (! index.isValid())
        return;

    KisPaintOpPreset* preset = static_cast<KisPaintOpPreset*>(index.internalPointer());

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText(), 2.0));
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->setPen(QPen(option.palette.text(), 2.0));

    }

    QImage preview = preset->image();

    if(preview.isNull()) {
        return;
    }

    QRect paintRect = option.rect.adjusted(2, 2, -2, -2);
    if (!m_showText) {
        painter->drawImage(paintRect.x(), paintRect.y(),
                           preview.scaled(paintRect.size(), Qt::IgnoreAspectRatio));
    } else {
        QSize pixSize(paintRect.height(), paintRect.height());
        painter->drawImage(paintRect.x(), paintRect.y(),
                           preview.scaled(pixSize, Qt::KeepAspectRatio));

        painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, preset->name());
    }

    if (!preset->settings() || !preset->settings()->isValid()) {
        const KIcon icon(koIconName("edit-delete"));
        icon.paint(painter, QRect(paintRect.x() + paintRect.height() - 25, paintRect.y() + paintRect.height() - 25, 25, 25));
    }
}

class KisPresetProxyAdapter : public KoResourceServerAdapter<KisPaintOpPreset>
{
    static bool compareKoResources(const KoResource* a, const KoResource* b)
    {
        return a->name() < b->name();
    }

public:
    KisPresetProxyAdapter(KoResourceServer< KisPaintOpPreset >* resourceServer)
        : KoResourceServerAdapter<KisPaintOpPreset>(resourceServer), m_filterNames(false)
    {
        m_showAll = KisConfig().presetShowAllMode();
    }
    virtual ~KisPresetProxyAdapter() {}

    virtual QList< KoResource* > resources() {
        if( ! resourceServer() )
            return QList<KoResource*>();

        QList<KisPaintOpPreset*> serverResources = resourceServer()->resources();

        QList<KoResource*> resources;
        foreach( KisPaintOpPreset* resource, serverResources ) {
            if(filterAcceptsPreset(resource) || (m_filterNames && m_filteredNames.contains(resource->filename())) ) {
                resources.append( resource );
            }
        }

        qSort(resources.begin(), resources.end(), KisPresetProxyAdapter::compareKoResources);
        return resources;
    }

    bool filterAcceptsPreset(KisPaintOpPreset* preset) const
    {
        if(m_paintopID.id().isEmpty())
            return true;

        return ((preset->paintOp() == m_paintopID || m_showAll) &&
                preset->name().contains(m_nameFilter, Qt::CaseInsensitive) &&
                (!m_filterNames || m_filteredNames.contains(preset->name())));
    }

    ///Set id for paintop to be accept by the proxy model, if not filter is set all
    ///presets will be shown.
    void setPresetFilter(const KoID &paintopID)
    {
        m_paintopID = paintopID;
    }

    KoID presetFilter()
    {
        return m_paintopID;
    }

    /// Set a filter for preset name, only presets with name containing the string will be shown
    void setPresetNameFilter(const QString &nameFilter)
    {
        m_nameFilter = nameFilter;
    }

    void setFilteredNames(const QStringList filteredNames)
    {
        m_filteredNames = filteredNames;
    }

    void setFilterNames(bool filterNames)
    {
        m_filterNames = filterNames;
    }

    void setShowAll(bool show)
    {
        m_showAll = show;
    }

    bool showAll()
    {
        return m_showAll;
    }


    ///Resets the model connected to the adapter
    void invalidate() {
        emitRemovingResource(0);
    }

private:
    KoID m_paintopID;
    QString m_nameFilter;
    bool m_showAll;
    bool m_filterNames;
    QStringList m_filteredNames;
};

KisPresetChooser::KisPresetChooser(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);
    KoResourceServer<KisPaintOpPreset> * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();
    m_presetProxy = new KisPresetProxyAdapter(rserver);
    m_chooser = new KoResourceItemChooser(m_presetProxy, this);
    QString knsrcFile = "kritapresets.knsrc";
    m_chooser->setKnsrcFile(knsrcFile);
    m_chooser->showGetHotNewStuff(true, true);
    m_chooser->setColumnCount(10);
    m_chooser->setRowHeight(50);
    m_delegate = new KisPresetDelegate(this);
    m_chooser->setItemDelegate(m_delegate);
    layout->addWidget(m_chooser);

    connect(m_chooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(resourceSelected(KoResource*)));

    m_mode = THUMBNAIL;
    updateViewSettings();
}

KisPresetChooser::~KisPresetChooser()
{
}

void KisPresetChooser::setPresetFilter(const KoID& paintopID)
{
    KoID oldFilter = m_presetProxy->presetFilter();
    m_presetProxy->setPresetFilter(paintopID);
    if(oldFilter.id() != paintopID.id() && !m_presetProxy->showAll()) {
        m_presetProxy->invalidate();
        updateViewSettings();
    }
}

void KisPresetChooser::setFilteredNames(const QStringList filteredNames)
{
    m_presetProxy->setFilterNames(true);
    m_presetProxy->setFilteredNames(filteredNames);
    m_presetProxy->invalidate();
    updateViewSettings();
}

void KisPresetChooser::searchTextChanged(const QString& searchString)
{
    if(searchString.isEmpty()) {
        m_presetProxy->setFilterNames(false);
    }
    m_presetProxy->setPresetNameFilter(searchString);
    m_presetProxy->invalidate();
    updateViewSettings();
}

void KisPresetChooser::returnKeyPressed(QString lineEditText)
{
    m_presetProxy->setFilterNames(true);
    m_presetProxy->setFilteredNames(m_chooser->getTaggedResourceFileNames(lineEditText));
    m_presetProxy->invalidate();
    updateViewSettings();
}

QStringList KisPresetChooser::getTagNamesList(const QString& searchString)
{
    return m_chooser->getTagNamesList(searchString);
}

void KisPresetChooser::setShowAll(bool show)
{
    m_presetProxy->setShowAll(show);
    m_presetProxy->invalidate();
    updateViewSettings();
}

void KisPresetChooser::showButtons(bool show)
{
    m_chooser->showButtons(show);
}

void KisPresetChooser::setViewMode(KisPresetChooser::ViewMode mode)
{
    m_mode = mode;
    updateViewSettings();
}

void KisPresetChooser::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateViewSettings();
}

void KisPresetChooser::updateViewSettings()
{
    if (m_mode == THUMBNAIL) {
        int resourceCount = m_presetProxy->resources().count();
        int width = m_chooser->viewSize().width();
        int maxColums = width/50;
        int cols = width/100 + 1;
        while(cols <= maxColums) {
            int size = width/cols;
            int rows = ceil(resourceCount/(double)cols);
            if(rows*size < (m_chooser->viewSize().height()-5)) {
                break;
            }
            cols++;
        }
        m_chooser->setRowHeight(floor((double)width/cols));
        m_chooser->setColumnCount(cols);
        m_delegate->setShowText(false);
    } else if (m_mode == DETAIL) {
        m_chooser->setColumnCount(1);
        m_delegate->setShowText(true);
    } else if (m_mode == STRIP) {
        m_chooser->setRowCount(1);
        // An offset of 7 keeps the cell exactly square, TODO: use constants, not hardcoded numbers
        m_chooser->setColumnWidth(m_chooser->viewSize().height() - 7);
        m_delegate->setShowText(false);
    }

}

KoResource* KisPresetChooser::currentResource()
{
    return m_chooser->currentResource();
}

void KisPresetChooser::showTaggingBar( bool showSearchBar, bool showOpBar )
{
    m_chooser->showTaggingBar(showSearchBar,showOpBar);
}

#include "kis_preset_chooser.moc"

