/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_equalizer_widget.h"

#include <QHBoxLayout>

#include "kis_equalizer_column.h"
#include "kis_signal_compressor.h"

#include "kis_debug.h"


struct KisEqualizerWidget::Private
{
    Private()
        : maxDistance(0),
          updateCompressor(300, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    QMap<int, KisEqualizerColumn*> columns;
    int maxDistance;
    KisSignalCompressor updateCompressor;
};

KisEqualizerWidget::KisEqualizerWidget(int maxDistance, QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->maxDistance = maxDistance;

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);

    for (int i = -m_d->maxDistance; i <= m_d->maxDistance; i++) {
        KisEqualizerColumn *c = new KisEqualizerColumn(this, i, QString::number(i));
        layout->addWidget(c, i == 0 ? 2 : 1);

        if (i == m_d->maxDistance) {
            c->setRightmost(true);
        }

        m_d->columns.insert(i, c);

        connect(c, SIGNAL(sigColumnChanged(int, bool, int)),
                &m_d->updateCompressor, SLOT(start()));
    }

    connect(&m_d->updateCompressor, SIGNAL(timeout()), SIGNAL(sigConfigChanged()));

    setLayout(layout);
}

KisEqualizerWidget::~KisEqualizerWidget()
{
}

KisEqualizerWidget::EqualizerValues KisEqualizerWidget::getValues() const
{
    EqualizerValues v;
    v.maxDistance = m_d->maxDistance;

    for (int i = -m_d->maxDistance; i <= m_d->maxDistance; i++) {
        v.value.insert(i, m_d->columns[i]->value());
        v.state.insert(i, m_d->columns[i]->state());
    }

    return v;
}

void KisEqualizerWidget::setValues(const EqualizerValues &v)
{
    for (int i = -m_d->maxDistance; i <= m_d->maxDistance; i++) {
        if (qAbs(i) <= v.maxDistance) {
            m_d->columns[i]->setState(v.state[i]);
            m_d->columns[i]->setValue(v.value[i]);
        } else {
            m_d->columns[i]->setState(false);
        }
    }
}
