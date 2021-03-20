/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    m_lblName->setTextElideMode(Qt::ElideMiddle);
    m_lblName->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

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
        if (rserver->resourceCount() > 0) {
            KisSignalsBlocker blocker(m_itemChooser);
            m_itemChooser->setCurrentResource(rserver->firstResource());
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
    KisSeExprScriptSP pattern = resource.staticCast<KisSeExprScript>();
    m_lblName->setText(QString("%1").arg(i18n(pattern->name().toUtf8().replace("_", " "))));
}
