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

#include <kis_config.h>
#include <klocalizedstring.h>
#include <KisKineticScroller.h>

#include <KoIcon.h>
#include <KoResourceItemChooser.h>
#include <KoResourceModel.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceItemChooserSync.h>
#include "KoResourceItemView.h"

#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_preset.h>
#include "KisResourceServerProvider.h"
#include "kis_global.h"
#include "kis_slider_spin_box.h"
#include "kis_config_notifier.h"
#include <kis_icon.h>

/// The resource item delegate for rendering the resource preview
class KisPresetDelegate : public QAbstractItemDelegate
{
public:
    KisPresetDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent), m_showText(false), m_useDirtyPresets(false) {}
    ~KisPresetDelegate() override {}
    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }

    void setShowText(bool showText) {
        m_showText = showText;
    }

    void setUseDirtyPresets(bool value) {
        m_useDirtyPresets = value;
    }

private:
    bool m_showText;
    bool m_useDirtyPresets;
};

void KisPresetDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (! index.isValid())
        return;

    KisPaintOpPreset* preset = static_cast<KisPaintOpPreset*>(index.internalPointer());

    QImage preview = preset->image();

    if(preview.isNull()) {
        return;
    }

    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);
    if (!m_showText) {
        painter->drawImage(paintRect.x(), paintRect.y(),
                           preview.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    } else {
        QSize pixSize(paintRect.height(), paintRect.height());
        painter->drawImage(paintRect.x(), paintRect.y(),
                           preview.scaled(pixSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // Put an asterisk after the preset if it is dirty. This will help in case the pixmap icon is too small
         QString dirtyPresetIndicator = QString("");
        if (m_useDirtyPresets && preset->isDirty()) {
            dirtyPresetIndicator = QString("*");
        }

        qreal brushSize = preset->settings()->paintOpSize();
        QString brushSizeText;

        // Disable displayed decimal precision beyond a certain brush size
        if (brushSize < 100) {
            brushSizeText = QString::number(brushSize, 'g', 3);
        } else {
            brushSizeText = QString::number(brushSize, 'f', 0);
        }

        painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, brushSizeText); // brush size

        QString presetDisplayName = preset->name().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(pixSize.width() + 40, option.rect.y() + option.rect.height() - 10, presetDisplayName.append(dirtyPresetIndicator));

    }
    if (m_useDirtyPresets && preset->isDirty()) {
        const QIcon icon = KisIconUtils::loadIcon(koIconName("dirty-preset"));
        QPixmap pixmap = icon.pixmap(QSize(15,15));
        painter->drawPixmap(paintRect.x() + 3, paintRect.y() + 3, pixmap);
    }

    if (!preset->settings() || !preset->settings()->isValid()) {
        const QIcon icon = KisIconUtils::loadIcon("broken-preset");
        icon.paint(painter, QRect(paintRect.x() + paintRect.height() - 25, paintRect.y() + paintRect.height() - 25, 25, 25));
    }
    if (option.state & QStyle::State_Selected) {
        painter->setCompositionMode(QPainter::CompositionMode_HardLight);
        painter->setOpacity(1.0);
        painter->fillRect(option.rect, option.palette.highlight());

        // highlight is not strong enough to pick out preset. draw border around it.
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(QPen(option.palette.highlight(), 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
        QRect selectedBorder = option.rect.adjusted(2 , 2, -2, -2); // constrict the rectangle so it doesn't bleed into other presets
        painter->drawRect(selectedBorder);
       }
    painter->restore();
}

class KisPresetProxyAdapter : public KisPaintOpPresetResourceServerAdapter
{

public:
    KisPresetProxyAdapter(KisPaintOpPresetResourceServer* resourceServer)
        : KisPaintOpPresetResourceServerAdapter(resourceServer)
    {
        setSortingEnabled(true);
    }
    ~KisPresetProxyAdapter() override {}

    QList< KoResource* > resources() override {

        QList<KoResource*> serverResources =
            KisPaintOpPresetResourceServerAdapter::resources();

        if (m_paintopID.isEmpty()) {
            return serverResources;
        }

        QList<KoResource*> resources;
        Q_FOREACH (KoResource *resource, serverResources) {
            KisPaintOpPreset *preset = dynamic_cast<KisPaintOpPreset*>(resource);

            if (preset && preset->paintOp().id() == m_paintopID) {
                resources.append(preset);
            }
        }
        return resources;
    }

    ///Set id for paintop to be accept by the proxy model, if not filter is set all
    ///presets will be shown.
    void setPresetFilter(const QString& paintOpId)
    {
        m_paintopID = paintOpId;
        invalidate();
    }

    ///Resets the model connected to the adapter
    void invalidate() {
        emitRemovingResource(0);
    }

    QString currentPaintOpId() const {
        return m_paintopID;
    }

private:
    QString m_paintopID;
};

KisPresetChooser::KisPresetChooser(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);
    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setMargin(0);
    KisPaintOpPresetResourceServer * rserver = KisResourceServerProvider::instance()->paintOpPresetServer();

    m_adapter = QSharedPointer<KoAbstractResourceServerAdapter>(new KisPresetProxyAdapter(rserver));

    m_chooser = new KoResourceItemChooser(m_adapter, this);
    m_chooser->setObjectName("ResourceChooser");
    m_chooser->setColumnCount(10);
    m_chooser->setRowHeight(50);
    m_delegate = new KisPresetDelegate(this);
    m_chooser->setItemDelegate(m_delegate);
    m_chooser->setSynced(true);
    layout->addWidget(m_chooser);

    {
        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this->itemChooser()->itemView());
        if (scroller) {
            connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                    this, SLOT(slotScrollerStateChanged(QScroller::State)));
        }
    }
    connect(m_chooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(resourceSelected(KoResource*)));
    connect(m_chooser, SIGNAL(resourceClicked(KoResource*)),
            this, SIGNAL(resourceClicked(KoResource*)));

    m_mode = THUMBNAIL;

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
            SLOT(notifyConfigChanged()));


    notifyConfigChanged();
}

KisPresetChooser::~KisPresetChooser()
{
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

void KisPresetChooser::notifyConfigChanged()
{
    KisConfig cfg(true);
    m_delegate->setUseDirtyPresets(cfg.useDirtyPresets());
    setIconSize(cfg.presetIconSize());

    updateViewSettings();
}

void KisPresetChooser::updateViewSettings()
{
    if (m_mode == THUMBNAIL) {
        m_chooser->setSynced(true);
        m_delegate->setShowText(false);
    } else if (m_mode == DETAIL) {
        m_chooser->setSynced(false);
        m_chooser->setColumnCount(1);
        m_chooser->setColumnWidth(m_chooser->width());

        KoResourceItemChooserSync* chooserSync = KoResourceItemChooserSync::instance();
        m_chooser->setRowHeight(chooserSync->baseLength());
        m_delegate->setShowText(true);
    } else if (m_mode == STRIP) {
        m_chooser->setSynced(false);
        m_chooser->setRowCount(1);
        m_chooser->itemView()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_chooser->itemView()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        // An offset of 7 keeps the cell exactly square, TODO: use constants, not hardcoded numbers
        m_chooser->setColumnWidth(m_chooser->viewSize().height() - 7);
        m_delegate->setShowText(false);
    }
}

void KisPresetChooser::setCurrentResource(KoResource *resource)
{
    /**
     * HACK ALERT: here we use a direct call to an adapter to notify the view
     *             that the preset might have changed its dirty state. This state
     *             doesn't affect the filtering so the server's cache must not be
     *             invalidated!
     *
     *             Ideally, we should call some method of KoResourceServer instead,
     *             but it seems like a bit too much effort for such a small fix.
     */
    if (resource == currentResource()) {
        KisPresetProxyAdapter *adapter = static_cast<KisPresetProxyAdapter*>(m_adapter.data());
        KisPaintOpPreset *preset = dynamic_cast<KisPaintOpPreset*>(resource);
        if (preset) {
            adapter->resourceChangedNoCacheInvalidation(preset);
        }
    }

    m_chooser->setCurrentResource(resource);
}

KoResource* KisPresetChooser::currentResource() const
{
    return m_chooser->currentResource();
}

void KisPresetChooser::showTaggingBar(bool show)
{
    m_chooser->showTaggingBar(show);
}

KoResourceItemChooser *KisPresetChooser::itemChooser()
{
    return m_chooser;
}


void KisPresetChooser::setPresetFilter(const QString& paintOpId)
{
    KisPresetProxyAdapter *adapter = static_cast<KisPresetProxyAdapter*>(m_adapter.data());

    if (adapter->currentPaintOpId() != paintOpId) {
        adapter->setPresetFilter(paintOpId);
        updateViewSettings();
    }
}

void KisPresetChooser::setIconSize(int newSize)
{
    KoResourceItemChooserSync* chooserSync = KoResourceItemChooserSync::instance();
    chooserSync->setBaseLength(newSize);
    updateViewSettings();
}

int KisPresetChooser::iconSize()
{
    KoResourceItemChooserSync* chooserSync = KoResourceItemChooserSync::instance();

    return chooserSync->baseLength();
}



void KisPresetChooser::saveIconSize()
{
    // save icon size
    KisConfig cfg(false);
    cfg.setPresetIconSize(iconSize());
}

void KisPresetChooser::slotScrollerStateChanged(QScroller::State state)
{
    KisKineticScroller::updateCursor(this, state);
}
