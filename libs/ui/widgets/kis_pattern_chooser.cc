/*
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_pattern_chooser.h"

#include <math.h>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QShowEvent>

#include <klocalizedstring.h>
#include <KisResourceItemChooser.h>
#include <KoResourceServerProvider.h>

#include "kis_signals_blocker.h"

#include "kis_global.h"
#include <kis_config.h>
#include <resources/KoPattern.h>

#include <ksqueezedtextlabel.h>

KisPatternChooser::KisPatternChooser(QWidget *parent)
    : QFrame(parent)
{
    m_lblName = new KSqueezedTextLabel(this);
    m_lblName->setTextElideMode(Qt::ElideMiddle);
    m_lblName->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    m_itemChooser = new KisResourceItemChooser(ResourceType::Patterns, true, this);
    m_itemChooser->setPreviewTiled(true);
    m_itemChooser->setPreviewOrientation(Qt::Horizontal);
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setSynced(true);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SLOT(update(KoResourceSP )));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResourceSP )),
            this, SIGNAL(resourceSelected(KoResourceSP )));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_lblName);
    mainLayout->addWidget(m_itemChooser, 10);
}

KisPatternChooser::~KisPatternChooser()
{
}

KoResourceSP  KisPatternChooser::currentResource()
{
    if (!m_itemChooser->currentResource()) {
        KoResourceServer<KoPattern> * rserver = KoResourceServerProvider::instance()->patternServer();
        if (rserver->resourceCount() > 0) {
            KisSignalsBlocker blocker(m_itemChooser);
            m_itemChooser->setCurrentResource(rserver->firstResource());
        }
    }
    return m_itemChooser->currentResource();
}

void KisPatternChooser::setCurrentPattern(KoResourceSP resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisPatternChooser::setCurrentItem(int row)
{
    m_itemChooser->setCurrentItem(row);
    if (currentResource()) {
        update(currentResource());
    }
}

void KisPatternChooser::setPreviewOrientation(Qt::Orientation orientation)
{
    m_itemChooser->setPreviewOrientation(orientation);
}

void KisPatternChooser::update(KoResourceSP resource)
{
    KoPatternSP pattern = resource.staticCast<KoPattern>();
    m_lblName->setText(QString("%1 (%2 x %3)").arg(i18n(pattern->name().toUtf8().data())).arg(pattern->width()).arg(pattern->height()));
}

void KisPatternChooser::setGrayscalePreview(bool grayscale)
{
    m_itemChooser->setGrayscalePreview(grayscale);
}


