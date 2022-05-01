/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *  SPDX-FileCopyrightText: 2022 Bourumir Wyngs <bourumir.wyngs@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_color_history.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_canvas_resource_provider.h"

#include <KoCompositeOpRegistry.h>

#include <QToolButton>
#include <QList>
#include <kis_icon.h>
#include "KisDocument.h"

KisColorHistory::KisColorHistory(QWidget *parent)
    : KisColorPatches("lastUsedColors", parent)
    , m_document(0)
    , m_resourceProvider(0)
{
    m_clearButton = new QToolButton(this);
    m_clearButton->setIcon(KisIconUtils::loadIcon("dialog-cancel-16"));
    m_clearButton->setToolTip(i18n("Clear all color history"));
    m_clearButton->setAutoRaise(true);
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearColorHistory()));

    setAdditionalButtons({m_clearButton});
}

void KisColorHistory::unsetCanvas()
{
    if (m_resourceProvider && m_document) {
        // Remember color history so we can use it as default for new image or save on shutdown
        m_resourceProvider->setLastColorHistory(m_document->colorHistory());
    }
    m_resourceProvider = 0;
    m_document = 0;

    KisColorPatches::unsetCanvas();
}

void KisColorHistory::setCanvas(KisCanvas2 *canvas)
{
    if (!canvas) return;

    KisColorPatches::setCanvas(canvas);

    if (m_resourceProvider) {
        m_resourceProvider->disconnect(this);
    }

    m_resourceProvider = canvas->imageView()->resourceProvider();
    m_document = canvas->viewManager()->document();

    if (m_resourceProvider && m_document) {
        // For new document, or if loaded from non .kra file, inherit the recent history if available
        if (m_document->colorHistory().empty()) {
            QList<KoColor> existingColors = colors();
            if (existingColors.empty()) {
                // Use last color history if otherwise it would be empty (new start).
                m_document->setColorHistory(m_resourceProvider->lastColorHistory());
            } else {
                // Try to stick with what the user has seen the last
                m_document->setColorHistory(existingColors);
            }
        }

        connect(m_resourceProvider, SIGNAL(sigFGColorUsed(KoColor)),
                this, SLOT(addColorToHistory(KoColor)), Qt::UniqueConnection);

        setColors(m_document->colorHistory());
    }
}

KisColorSelectorBase* KisColorHistory::createPopup() const
{
    KisColorHistory* ret = new KisColorHistory();
    ret->setCanvas(m_canvas);
    ret->setColors(colors());
    return ret;
}

void KisColorHistory::addColorToHistory(const KoColor& color)
{
    // don't add color in erase mode. See https://bugs.kde.org/show_bug.cgi?id=298940
    if (m_resourceProvider && m_resourceProvider->currentCompositeOp() == COMPOSITE_ERASE) return;

    if (m_document) {
        QList<KoColor> colorHistory = m_document->colorHistory();

        colorHistory.removeAll(color);
        colorHistory.prepend(color);

        //the history holds 200 colors, but not all are displayed
        if (colorHistory.size() > 200) {
            colorHistory.removeLast();
        }
        m_document->setColorHistory(colorHistory);
        setColors(colorHistory);
    }
}

void KisColorHistory::clearColorHistory() {
    QList<KoColor> empty;
    m_document->setColorHistory(empty);
    setColors(empty);
}
