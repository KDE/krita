/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_equalizer_widget.h"

#include <QMouseEvent>
#include <QApplication>
#include <QHBoxLayout>

#include "kis_equalizer_column.h"
#include "kis_signal_compressor.h"

#include "KisAnimTimelineColors.h"

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
    layout->setMargin(0);

    for (int i = -m_d->maxDistance; i <= m_d->maxDistance; i++) {
        KisEqualizerColumn *c = new KisEqualizerColumn(this, i, QString::number(i));
        layout->addWidget(c, i == 0 ? 2 : 1);

        if (i == m_d->maxDistance) {
            c->setRightmost(true);
        }

        m_d->columns.insert(i, c);

        connect(c, SIGNAL(sigColumnChanged(int,bool,int)),
                &m_d->updateCompressor, SLOT(start()));
    }

    connect(&m_d->updateCompressor, SIGNAL(timeout()), SIGNAL(sigConfigChanged()));
    connect(m_d->columns[0], SIGNAL(sigColumnChanged(int,bool,int)), this, SLOT(slotMasterColumnChanged(int,bool,int)));
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
            m_d->columns[i]->setValue(v.value[i]);
            m_d->columns[i]->setState(v.state[i]);
        } else {
            m_d->columns[i]->setState(false);
        }
    }
}

void KisEqualizerWidget::toggleMasterSwitch()
{
    const bool currentState = m_d->columns[0]->state();
    m_d->columns[0]->setState(!currentState);
}

void KisEqualizerWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    const QSize newSize = m_d->columns[1]->size();

    QFont font =
        KisAnimTimelineColors::instance()->getOnionSkinsFont(
            QString::number(100), newSize);

    if (font.pointSize() != this->font().pointSize()) {
        setFont(font);

        for (int i = -m_d->maxDistance; i <= m_d->maxDistance; i++) {
            m_d->columns[i]->setFont(font);
        }

    }
}

void KisEqualizerWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (!(ev->modifiers() & Qt::ShiftModifier)) return;

    QPoint globalPos = ev->globalPos();
    QWidget *w = qApp->widgetAt(globalPos);

    if (w && w->inherits("QAbstractSlider")) {
        QMouseEvent newEv(ev->type(),
                          w->mapFromGlobal(globalPos),
                          globalPos,
                          ev->button(),
                          ev->buttons(),
                          ev->modifiers() & ~Qt::ShiftModifier);
        qApp->sendEvent(w, &newEv);
    }
}

void KisEqualizerWidget::slotMasterColumnChanged(int, bool state, int)
{
    for (int i = 1; i <= m_d->maxDistance; i++) {
        m_d->columns[i]->setForceDisabled(!state);
        m_d->columns[-i]->setForceDisabled(!state);
    }
}
