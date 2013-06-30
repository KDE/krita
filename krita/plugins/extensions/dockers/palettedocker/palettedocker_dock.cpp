/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#include "palettedocker_dock.h"

#include <QPainter>
#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <klocale.h>
#include <kcolordialog.h>

#include <KoIcon.h>
#include <KoCanvasBase.h>
#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>
#include <KoCanvasResourceManager.h>

#include <kis_view2.h>
#include <kis_canvas2.h>
#include <kis_layer.h>
#include <kis_node_manager.h>
#include <kis_config.h>
#include <kis_workspace_resource.h>
#include <kis_canvas_resource_provider.h>

#include "palettemodel.h"
#include "colorsetchooser.h"
#include "ui_wdgpalettedock.h"

/// The resource item delegate for rendering the resource preview
class PaletteDelegate : public QAbstractItemDelegate
{
public:
    PaletteDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent), m_showText(false) {}
    virtual ~PaletteDelegate() {}
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

void PaletteDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();

    if (! index.isValid())
        return;

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText(), 2.0));
        painter->fillRect(option.rect, option.palette.highlight());
    } else {
        painter->setPen(QPen(option.palette.text(), 2.0));

    }
    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);
    QBrush brush = qVariantValue<QBrush>(index.data(Qt::BackgroundRole));
    painter->fillRect(paintRect, brush);
    painter->restore();
}


PaletteDockerDock::PaletteDockerDock( ) : QDockWidget(i18n("Palette"))
    , m_canvas(0)
    , m_wdgPaletteDock(new Ui_WdgPaletteDock())
    , m_currentColorSet(0)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_wdgPaletteDock->setupUi(mainWidget);
    m_wdgPaletteDock->bnAdd->setIcon(koIcon("list-add"));
    m_wdgPaletteDock->bnAdd->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnAddDialog->setIcon(koIcon("color"));
    m_wdgPaletteDock->bnAddDialog->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnRemove->setIcon(koIcon("list-remove"));
    m_wdgPaletteDock->bnRemove->setIconSize(QSize(16, 16));
    m_wdgPaletteDock->bnAdd->setEnabled(false);
    m_wdgPaletteDock->bnRemove->setEnabled(false);

    connect(m_wdgPaletteDock->bnAdd, SIGNAL(clicked(bool)), this, SLOT(addColorForeground()));
    connect(m_wdgPaletteDock->bnAddDialog, SIGNAL(clicked(bool)), this, SLOT(addColor()));
    connect(m_wdgPaletteDock->bnRemove, SIGNAL(clicked(bool)), this, SLOT(removeColor()));

    m_model = new PaletteModel(this);
    m_wdgPaletteDock->paletteView->setModel(m_model);
    m_wdgPaletteDock->paletteView->setShowGrid(false);
    m_wdgPaletteDock->paletteView->horizontalHeader()->setVisible(false);
    m_wdgPaletteDock->paletteView->verticalHeader()->setVisible(false);
    m_wdgPaletteDock->paletteView->setItemDelegate(new PaletteDelegate());
    m_wdgPaletteDock->paletteView->horizontalHeader()->setDefaultSectionSize(12);
    m_wdgPaletteDock->paletteView->verticalHeader()->setDefaultSectionSize(12);

    QPalette pal(palette());
    pal.setColor(QPalette::Base, pal.dark().color());
    m_wdgPaletteDock->paletteView->setAutoFillBackground(true);
    m_wdgPaletteDock->paletteView->setPalette(pal);
 
    connect(m_wdgPaletteDock->paletteView, SIGNAL(activated(QModelIndex)), this, SLOT(entrySelected(QModelIndex)));

    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    m_serverAdapter = new KoResourceServerAdapter<KoColorSet>(rServer, this);
    m_serverAdapter->connectToResourceServer();

    m_colorSetChooser = new ColorSetChooser();
    connect(m_colorSetChooser, SIGNAL(paletteSelected(KoColorSet*)), this, SLOT(setColorSet(KoColorSet*)));

    m_wdgPaletteDock->bnColorSets->setIcon(koIcon("document-multiple"));
    m_wdgPaletteDock->bnColorSets->setToolTip(i18n("Choose palette"));
    m_wdgPaletteDock->bnColorSets->setPopupWidget(m_colorSetChooser);

    KisConfig cfg;
    QString defaultPalette = cfg.defaultPalette();
    KoColorSet* defaultColorSet = rServer->resourceByName(defaultPalette);
    if (defaultColorSet) {
        setColorSet(defaultColorSet);
    }
}

PaletteDockerDock::~PaletteDockerDock()
{
    if (m_currentColorSet) {
        KisConfig cfg;
        cfg.setDefaultPalette(m_currentColorSet->name());
    }
}

void PaletteDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas && m_canvas->view()) {
        m_canvas->view()->nodeManager()->disconnect(m_model);
    }
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    KisView2* view = m_canvas->view();
    connect(view->resourceProvider(), SIGNAL(sigSavingWorkspace(KisWorkspaceResource*)), SLOT(saveToWorkspace(KisWorkspaceResource*)));
    connect(view->resourceProvider(), SIGNAL(sigLoadingWorkspace(KisWorkspaceResource*)), SLOT(loadFromWorkspace(KisWorkspaceResource*)));
}

void PaletteDockerDock::setColorSet(KoColorSet* colorSet)
{
    m_model->setColorSet(colorSet);
    if (colorSet->removable()) {
        m_wdgPaletteDock->bnAdd->setEnabled(true);
        m_wdgPaletteDock->bnRemove->setEnabled(false);
    } else {
        m_wdgPaletteDock->bnAdd->setEnabled(false);
        m_wdgPaletteDock->bnRemove->setEnabled(false);
    }
    m_currentColorSet = colorSet;
}

void PaletteDockerDock::addColorForeground()
{
    KoColorSetEntry newEntry;
    newEntry.color = m_canvas->resourceManager()->foregroundColor();
    m_currentColorSet->add(newEntry);
    m_currentColorSet->save();
    setColorSet(m_currentColorSet); // update model
}

void PaletteDockerDock::addColor()
{
    if (m_currentColorSet) {
        QColor color;
        int result = KColorDialog::getColor(color, m_canvas->resourceManager()->foregroundColor().toQColor());
        if (result == KColorDialog::Accepted) {
            KoColorSetEntry newEntry;
            newEntry.color = KoColor(color, KoColorSpaceRegistry::instance()->rgb8());
            m_currentColorSet->add(newEntry);
            m_currentColorSet->save();
            setColorSet(m_currentColorSet); // update model
        }
    }
}

void PaletteDockerDock::removeColor()
{
    QModelIndex index = m_wdgPaletteDock->paletteView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    int i = index.row()*m_model->columnCount()+index.column();
    KoColorSetEntry entry = m_currentColorSet->getColor(i);
    m_currentColorSet->remove(entry);
    m_currentColorSet->save();
    setColorSet(m_currentColorSet); // update model
}

void PaletteDockerDock::entrySelected(QModelIndex index)
{
    if (!index.isValid()) {
        return;
    }
    int i = index.row()*m_model->columnCount()+index.column();
    KoColorSetEntry entry = m_currentColorSet->getColor(i);
    if (m_canvas) {
        m_canvas->resourceManager()->setForegroundColor(entry.color);
    }
    if (m_currentColorSet->removable()) {
        m_wdgPaletteDock->bnRemove->setEnabled(true);
    }
}

void PaletteDockerDock::saveToWorkspace(KisWorkspaceResource* workspace)
{
    if (m_currentColorSet) {
        workspace->setProperty("palette", m_currentColorSet->name());
    }
}

void PaletteDockerDock::loadFromWorkspace(KisWorkspaceResource* workspace)
{
    if (workspace->hasProperty("palette")) {
        KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
        KoColorSet* colorSet = rServer->resourceByName(workspace->getString("palette"));
        if (colorSet) {
            setColorSet(colorSet);
        }
    }
}


#include "palettedocker_dock.moc"
