/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
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

#include <QPointer>
#include <QScopedPointer>
#include <QGridLayout>
#include <QSet>
#include <QStringList>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

#include <kis_icon.h>
#include <KoFileDialog.h>

#include <KisResourceModel.h>
#include <KisResourceItemListView.h>

#include <ui_WdgPaletteListWidget.h>
#include "KisPaletteListWidget.h"
#include "KisPaletteListWidget_p.h"

KisPaletteListWidget::KisPaletteListWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_WdgPaletteListWidget)
    , m_d(new KisPaletteListWidgetPrivate(this))
{
    m_d->allowModification = false;

    m_d->actAdd.reset(new QAction(KisIconUtils::loadIcon("list-add"),
                                  i18n("Add a new palette")));
    m_d->actRemove.reset(new QAction(KisIconUtils::loadIcon("list-remove"),
                                     i18n("Remove current palette")));
    m_d->actImport.reset(new QAction(KisIconUtils::loadIcon("document-import"),
                                     i18n("Import a new palette from file")));
    m_d->actExport.reset(new QAction(KisIconUtils::loadIcon("document-export"),
                                     i18n("Export current palette to file")));
    m_ui->setupUi(this);
    m_ui->bnAdd->setDefaultAction(m_d->actAdd.data());
    m_ui->bnRemove->setDefaultAction(m_d->actRemove.data());
    m_ui->bnImport->setDefaultAction(m_d->actImport.data());
    m_ui->bnExport->setDefaultAction(m_d->actExport.data());

    m_ui->bnAdd->setEnabled(false);
    m_ui->bnRemove->setEnabled(false);
    m_ui->bnImport->setEnabled(false);
    m_ui->bnExport->setEnabled(false);

    connect(m_d->actAdd.data(), SIGNAL(triggered()), SLOT(slotAdd()));
    connect(m_d->actRemove.data(), SIGNAL(triggered()), SLOT(slotRemove()));
    connect(m_d->actImport.data(), SIGNAL(triggered()), SLOT(slotImport()));
    connect(m_d->actExport.data(), SIGNAL(triggered()), SLOT(slotExport()));

    m_d->itemChooser->setItemDelegate(m_d->delegate.data());
    m_d->itemChooser->setRowHeight(40);
    m_d->itemChooser->itemView()->setViewMode(QListView::ListMode);
    m_d->itemChooser->showButtons(false);
    m_d->itemChooser->showTaggingBar(true);
    m_ui->viewPalette->setLayout(new QHBoxLayout(m_ui->viewPalette));
    m_ui->viewPalette->layout()->addWidget(m_d->itemChooser.data());

    connect(m_d->itemChooser.data(), SIGNAL(resourceSelected(KoResourceSP )), SLOT(slotPaletteResourceSelected(KoResourceSP )));
}

KisPaletteListWidget::~KisPaletteListWidget()
{ }

void KisPaletteListWidget::slotPaletteResourceSelected(KoResourceSP r)
{
    KoColorSetSP g = r.staticCast<KoColorSet>();
    emit sigPaletteSelected(g);
    if (!m_d->allowModification) { return; }
    if (g->isEditable()) {
        m_ui->bnRemove->setEnabled(true);
    } else {
        m_ui->bnRemove->setEnabled(false);
    }
}

void KisPaletteListWidget::slotAdd()
{
    if (!m_d->allowModification) { return; }
    emit sigAddPalette();
    m_d->itemChooser->setCurrentItem(m_d->itemChooser->rowCount() - 1);
}

void KisPaletteListWidget::slotRemove()
{
    if (!m_d->allowModification) { return; }
    if (m_d->itemChooser->currentResource()) {
        KoColorSetSP cs = m_d->itemChooser->currentResource().staticCast<KoColorSet>();
        emit sigRemovePalette(cs);
    }
    m_d->itemChooser->setCurrentItem(0);
}

void KisPaletteListWidget::slotImport()
{
    if (!m_d->allowModification) { return; }
    emit sigImportPalette();
    m_d->itemChooser->setCurrentItem(m_d->itemChooser->rowCount() - 1);
}

void KisPaletteListWidget::slotExport()
{
    if (!m_d->allowModification) { return; }
    emit sigExportPalette(m_d->itemChooser->currentResource().staticCast<KoColorSet>());
}

void KisPaletteListWidget::setAllowModification(bool allowModification)
{
    m_d->allowModification = allowModification;
    m_ui->bnAdd->setEnabled(allowModification);
    m_ui->bnImport->setEnabled(allowModification);
    m_ui->bnExport->setEnabled(allowModification);
    KoColorSetSP cs = m_d->itemChooser->currentResource().staticCast<KoColorSet>();
    m_ui->bnRemove->setEnabled(allowModification && cs && cs->isEditable());
}

/************************* KisPaletteListWidgetPrivate **********************/

KisPaletteListWidgetPrivate::KisPaletteListWidgetPrivate(KisPaletteListWidget *a_c)
    : c(a_c)
    , itemChooser(new KisResourceItemChooser(ResourceType::Palettes, false, a_c))
    , delegate(new Delegate(a_c))
{  }

KisPaletteListWidgetPrivate::~KisPaletteListWidgetPrivate()
{ }

/******************* KisPaletteListWidgetPrivate::Delegate ******************/

KisPaletteListWidgetPrivate::Delegate::Delegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{  }

KisPaletteListWidgetPrivate::Delegate::~Delegate()
{  }

void KisPaletteListWidgetPrivate::Delegate::paint(QPainter *painter,
                                                  const QStyleOptionViewItem &option,
                                                  const QModelIndex &index) const
{
    painter->save();
    if (!index.isValid())
        return;

    QImage preview = index.data(Qt::DecorationRole).value<QImage>();
    QString name = index.data(Qt::UserRole + KisResourceModel::Name).toString();

    QRect previewRect(option.rect.x() + 2,
                      option.rect.y() + 2,
                      option.rect.height() - 4,
                      option.rect.height() - 4);

    painter->drawImage(previewRect, preview);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->drawImage(previewRect, preview);
        painter->setPen(option.palette.highlightedText().color());
    } else {
        painter->setBrush(option.palette.text().color());
    }

    painter->drawText(option.rect.x() + previewRect.width() + 10,
                      option.rect.y() + painter->fontMetrics().ascent() + 5,
                      name);

    painter->restore();
}

inline QSize KisPaletteListWidgetPrivate::Delegate::sizeHint(const QStyleOptionViewItem & option,
                                                             const QModelIndex &) const
{
    return option.decorationSize;
}
