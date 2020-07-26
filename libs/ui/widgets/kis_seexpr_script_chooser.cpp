/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include "kis_seexpr_script_chooser.h"

#include <math.h>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QShowEvent>

#include <klocalizedstring.h>
#include <KoResourceServerProvider.h>

#include "kis_signals_blocker.h"

#include "kis_global.h"
#include <kis_config.h>
#include <resources/KisSeExprScript.h>

#include <ksqueezedtextlabel.h>

KisSeExprScriptChooser::KisSeExprScriptChooser(QWidget *parent)
    : QFrame(parent)
{
    m_lblName = new KSqueezedTextLabel(this);
    m_lblName->setTextElideMode(Qt::ElideLeft);

    m_itemChooser = new KisResourceItemChooser(ResourceType::SeExprScripts, true, this);
    m_itemChooser->setPreviewTiled(true);
    m_itemChooser->setPreviewOrientation(Qt::Horizontal);
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setSynced(true);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP)),
            this, SLOT(update(KoResourceSP)));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP)),
            this, SIGNAL(resourceSelected(KoResourceSP)));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_lblName);
    mainLayout->addWidget(m_itemChooser, 10);
}

KisSeExprScriptChooser::~KisSeExprScriptChooser()
{
}

KoResourceSP KisSeExprScriptChooser::currentResource()
{
    if (!m_itemChooser->currentResource()) {
        KoResourceServer<KisSeExprScript> *rserver = KoResourceServerProvider::instance()->seExprScriptServer();
        if (rserver->resources().size() > 0) {
            KisSignalsBlocker blocker(m_itemChooser);
            m_itemChooser->setCurrentResource(rserver->resources().first());
        }
    }
    return m_itemChooser->currentResource();
}

void KisSeExprScriptChooser::setCurrentScript(KoResourceSP resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisSeExprScriptChooser::setCurrentItem(int row)
{
    m_itemChooser->setCurrentItem(row);
    if (currentResource()) {
        update(currentResource());
    }
}

void KisSeExprScriptChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    m_itemChooser->setPreviewOrientation(orientation);
}

void KisSeExprScriptChooser::update(KoResourceSP resource)
{
    m_lblName->setFixedWidth(m_itemChooser->width());
    KisSeExprScriptSP pattern = resource.staticCast<KisSeExprScript>();
    m_lblName->setText(QString("%1").arg(i18n(pattern->name().toUtf8())));
}
