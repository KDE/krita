/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *                2004 Adrian Page     <adrian@pagenet.plus.com>
 *                2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_progress_widget.h"
#include <QDebug>
#include <QToolButton>
#include <QHBoxLayout>
#include <QKeyEvent>

#include <kiconloader.h>

#include <KoProgressUpdater.h>
#include <KoProgressBar.h>

#include <kis_progress_updater.h>

class EscapeButton : public QToolButton
{

public:

    EscapeButton(QWidget* parent)
            : QToolButton(parent) {
    }

    void keyReleaseEvent(QKeyEvent *e) {
        if (e->key() == Qt::Key_Escape) {
            emit clicked();
        }
    }

};

KisProgressWidget::KisProgressWidget(QWidget* parent)
        : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    m_cancelButton = new EscapeButton(this);
    m_cancelButton->setIcon(SmallIcon("process-stop"));
    connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));

    m_progressBar = new KoProgressBar(this);
    connect(m_progressBar, SIGNAL(done()), this, SLOT(hide()));
    layout->addWidget(m_progressBar);
    layout->addWidget(m_cancelButton);
}

KisProgressWidget::~KisProgressWidget()
{
    cancel();
}

KoProgressUpdater* KisProgressWidget::createUpdater(KoProgressUpdater::Mode mode)
{
    setVisible(this);
    KoProgressUpdater* updater = new KisProgressUpdater(this, m_progressBar, mode);
    return updater;
}

void KisProgressWidget::cancel()
{
    foreach(KoProgressUpdater* updater, m_activeUpdaters) {
        updater->cancel();
    }
}

void KisProgressWidget::detachUpdater(KoProgressUpdater* updater)
{
    m_activeUpdaters.removeOne(updater);
}

void KisProgressWidget::attachUpdater(KoProgressUpdater* updater)
{
    m_activeUpdaters << updater;
}

