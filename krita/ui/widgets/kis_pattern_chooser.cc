/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "widgets/kis_pattern_chooser.h"

#include <math.h>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QShowEvent>

#include <klocale.h>
#include <kfiledialog.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>

#include "KoColorSpace.h"

#include "kis_global.h"
#include "kis_pattern.h"
#include "kis_resource_server_provider.h"

KisPatternChooser::KisPatternChooser(QWidget *parent)
        : QFrame(parent)
{
    m_lbName = new QLabel(this);

    KoResourceServer<KisPattern> * rserver = KisResourceServerProvider::instance()->patternServer();
    KoAbstractResourceServerAdapter* adapter = new KoResourceServerAdapter<KisPattern>(rserver);
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->showPreview(true);
    m_itemChooser->setPreviewTiled(true);
    m_itemChooser->setPreviewOrientation(Qt::Horizontal);
    QString knsrcFile = "kritapatterns.knsrc";
    m_itemChooser->setKnsrcFile(knsrcFile);
    m_itemChooser->showGetHotNewStuff(true, true);
    m_itemChooser->showTaggingBar(true, true);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SLOT(update(KoResource *)));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SIGNAL(resourceSelected(KoResource *)));

    connect(m_itemChooser, SIGNAL(splitterMoved()),
            this, SLOT(updateItemSize()));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(m_itemChooser, 10);

    setLayout(mainLayout);
}

KisPatternChooser::~KisPatternChooser()
{
}

KoResource *  KisPatternChooser::currentResource()
{
    return m_itemChooser->currentResource();
}

void KisPatternChooser::setCurrentPattern(KoResource *resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisPatternChooser::setCurrentItem(int row, int column)
{
    m_itemChooser->setCurrentItem(row, column);
    if (currentResource()) {
        update(currentResource());
    }
}

void KisPatternChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    m_itemChooser->setPreviewOrientation(orientation);
}

void KisPatternChooser::update(KoResource * resource)
{
    KisPattern *pattern = static_cast<KisPattern *>(resource);

    QString text = QString("%1 (%2 x %3)").arg(i18n(pattern->name().toUtf8().data())).arg(pattern->width()).arg(pattern->height());
    m_lbName->setText(text);
}

void KisPatternChooser::setGrayscalePreview(bool grayscale)
{
    m_itemChooser->setGrayscalePreview(grayscale);
}

void KisPatternChooser::showEvent(QShowEvent*)
{
    updateItemSize();
}

void KisPatternChooser::updateItemSize()
{
    KisPattern* current = static_cast<KisPattern*>(currentResource());
    int width = m_itemChooser->viewSize().width();
    int cols = width/50 + 1;
    m_itemChooser->setRowHeight(floor((double)width/cols));
    m_itemChooser->setColumnCount(cols);
    //restore current pattern
    if(current) {
        setCurrentPattern(current);
    }
}

#include "kis_pattern_chooser.moc"

