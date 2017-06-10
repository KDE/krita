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

#include "kis_palette_view.h"

#include <QWheelEvent>
#include <QHeaderView>

#include "kis_palette_delegate.h"
#include "KisPaletteModel.h"
#include "kis_config.h"


struct KisPaletteView::Private
{
    KisPaletteModel *model = 0;
};


KisPaletteView::KisPaletteView(QWidget *parent)
    : KoTableView(parent),
      m_d(new Private)
{
    setShowGrid(false);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    setItemDelegate(new KisPaletteDelegate());

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    KisConfig cfg;
    QPalette pal(palette());
    pal.setColor(QPalette::Base, cfg.getMDIBackgroundColor());
    setAutoFillBackground(true);
    setPalette(pal);

    int defaultSectionSize = cfg.paletteDockerPaletteViewSectionSize();
    horizontalHeader()->setDefaultSectionSize(defaultSectionSize);
    verticalHeader()->setDefaultSectionSize(defaultSectionSize);
}

KisPaletteView::~KisPaletteView()
{
}

void KisPaletteView::setCrossedKeyword(const QString &value)
{
    KisPaletteDelegate *delegate =
        dynamic_cast<KisPaletteDelegate*>(itemDelegate());
    KIS_ASSERT_RECOVER_RETURN(delegate);

    delegate->setCrossedKeyword(value);
}

void KisPaletteView::setPaletteModel(KisPaletteModel *model)
{
    m_d->model = model;
    setModel(model);
}

KisPaletteModel* KisPaletteView::paletteModel() const
{
    return m_d->model;
}

void KisPaletteView::updateRows()
{
    for (int r=0; r<=m_d->model->rowCount(); r++) {
        QModelIndex index = m_d->model->index(r, 0);
        if (qVariantValue<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
            setSpan(r, 0, 1, m_d->model->columnCount());
            setRowHeight(r, this->fontMetrics().lineSpacing()+6);
        } else {
            if (columnSpan(r, 0)>1){
                setSpan(r, 0, 1, 1);
            }
        }
    }
}


void KisPaletteView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 7;
        int curSize = horizontalHeader()->sectionSize(0);
        int setSize = numSteps + curSize;

        if ( setSize >= 12 ) {
            horizontalHeader()->setDefaultSectionSize(setSize);
            verticalHeader()->setDefaultSectionSize(setSize);
            KisConfig cfg;
            cfg.setPaletteDockerPaletteViewSectionSize(setSize);
        }

        event->accept();
    } else {
        KoTableView::wheelEvent(event);
    }
}
