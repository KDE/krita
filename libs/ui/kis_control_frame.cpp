/*
 *  kis_control_frame.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_control_frame.h"

#include <stdlib.h>

#include <QApplication>
#include <QLayout>
#include <QTabWidget>
#include <QFrame>
#include <QWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QWidgetAction>
#include <QFontDatabase>

#include <klocalizedstring.h>
#include <QAction>
#include <kactioncollection.h>
#include <KoDualColorButton.h>
#include <resources/KoAbstractGradient.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>

#include <resources/KoPattern.h>
#include "KisResourceServerProvider.h"
#include "kis_canvas_resource_provider.h"

#include "widgets/kis_iconwidget.h"

#include "widgets/kis_gradient_chooser.h"
#include "KisViewManager.h"
#include "kis_config.h"
#include "kis_paintop_box.h"
#include "kis_custom_pattern.h"
#include "widgets/kis_pattern_chooser.h"
#include "kis_favorite_resource_manager.h"
#include "kis_display_color_converter.h"
#include <kis_canvas2.h>


KisControlFrame::KisControlFrame(KisViewManager *view, QWidget *parent, const char* name)
    : QObject(view)
    , m_viewManager(view)
    , m_patternWidget(0)
    , m_gradientWidget(0)
    , m_patternChooserPopup(0)
    , m_gradientChooserPopup(0)
    , m_paintopBox(0)
    , m_checkersPainter(4)
{
    setObjectName(name);
    m_font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    m_patternWidget = new KisIconWidget(parent, ResourceType::Patterns);
    m_patternWidget->setToolTip(i18n("Fill Patterns"));
    m_patternWidget->setFixedSize(32, 32);

    m_gradientWidget = new KisIconWidget(parent, ResourceType::Gradients);
    m_gradientWidget->setToolTip(i18n("Fill Gradients"));
    m_gradientWidget->setFixedSize(32, 32);
}

void KisControlFrame::setup(QWidget *parent)
{
    createPatternsChooser(m_viewManager);
    createGradientsChooser(m_viewManager);

    QWidgetAction *action  = new QWidgetAction(this);
    action->setText(i18n("&Patterns"));
    m_viewManager->actionCollection()->addAction(ResourceType::Patterns, action);
    action->setDefaultWidget(m_patternWidget);

    action = new QWidgetAction(this);
    action->setText(i18n("&Gradients"));
    m_viewManager->actionCollection()->addAction(ResourceType::Gradients, action);
    action->setDefaultWidget(m_gradientWidget);


    // XXX: KOMVC we don't have a canvas here yet, needs a setImageView
    const KoColorDisplayRendererInterface *displayRenderer = \
        KisDisplayColorConverter::dumbConverterInstance()->displayRendererInterface();
    m_dual = new KoDualColorButton(m_viewManager->canvasResourceProvider()->fgColor(),
                                                     m_viewManager->canvasResourceProvider()->bgColor(), displayRenderer,
                                                     m_viewManager->mainWindow(), m_viewManager->mainWindow());
    m_dual->setPopDialog(true);
    action = new QWidgetAction(this);
    action->setText(i18n("&Choose foreground and background colors"));
    m_viewManager->actionCollection()->addAction("dual", action);
    action->setDefaultWidget(m_dual);
    connect(m_dual, SIGNAL(foregroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(m_dual, SIGNAL(backgroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetBGColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_dual, SLOT(setBackgroundColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_dual, SLOT(setForegroundColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_gradientWidget, SLOT(update()));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_gradientWidget, SLOT(update()));
    m_dual->setFixedSize(28, 28);
    connect(m_viewManager, SIGNAL(viewChanged()), SLOT(slotUpdateDisplayRenderer()));

    m_paintopBox = new KisPaintopBox(m_viewManager, parent, "paintopbox");

    action = new QWidgetAction(this);
    action->setText(i18n("&Painter's Tools"));
    m_viewManager->actionCollection()->addAction("paintops", action);
    action->setDefaultWidget(m_paintopBox);
}

void KisControlFrame::slotUpdateDisplayRenderer()
{
    if (m_viewManager->canvasBase()){
        m_dual->setDisplayRenderer(m_viewManager->canvasBase()->displayColorConverter()->displayRendererInterface());
        m_dual->setColorSpace(m_viewManager->canvasBase()->image()->colorSpace());
        m_viewManager->canvasBase()->image()->disconnect(m_dual);
        connect(m_viewManager->canvasBase()->image(), SIGNAL(sigColorSpaceChanged(const KoColorSpace*)), m_dual, SLOT(setColorSpace(const KoColorSpace*)), Qt::UniqueConnection);
    } else if (m_viewManager->viewCount()==0) {
        m_dual->setDisplayRenderer();
    }
}

void KisControlFrame::slotSetPattern(KoPatternSP pattern)
{
    m_patternWidget->setThumbnail(pattern->image());
    m_patternChooser->setCurrentPattern(pattern);
}

void KisControlFrame::slotSetGradient(KoAbstractGradientSP gradient)
{
    const QSize iconSize = m_gradientWidget->preferredIconSize();

    QImage icon(iconSize, QImage::Format_ARGB32);

    {
        QPainter gc(&icon);
        m_checkersPainter.paint(gc, QRect(QPoint(), iconSize));
        gc.drawImage(QPoint(),
                     gradient->generatePreview(iconSize.width(), iconSize.height(),
                                               m_viewManager->canvasResourceProvider()->
                                               resourceManager()->canvasResourcesInterface()));
    }


    m_gradientWidget->setThumbnail(icon);
}

void KisControlFrame::createPatternsChooser(KisViewManager * view)
{
    if (m_patternChooserPopup) delete m_patternChooserPopup;
    m_patternChooserPopup = new QWidget(m_patternWidget);
    m_patternChooserPopup->setObjectName("pattern_chooser_popup");
    QHBoxLayout * l2 = new QHBoxLayout(m_patternChooserPopup);
    l2->setObjectName("patternpopuplayout");

    m_patternsTab = new QTabWidget(m_patternChooserPopup);
    m_patternsTab->setObjectName("patternstab");
    m_patternsTab->setFocusPolicy(Qt::NoFocus);
    m_patternsTab->setFont(m_font);
    l2->addWidget(m_patternsTab);

    m_patternChooser = new KisPatternChooser(m_patternChooserPopup);
    m_patternChooser->setFont(m_font);
    QWidget *patternChooserPage = new QWidget(m_patternChooserPopup);
    QHBoxLayout *patternChooserPageLayout  = new QHBoxLayout(patternChooserPage);
    patternChooserPageLayout->addWidget(m_patternChooser);
    m_patternsTab->addTab(patternChooserPage, i18n("Patterns"));

    KisCustomPattern* customPatterns = new KisCustomPattern(0, "custompatterns",
                                                            i18n("Custom Pattern"), m_viewManager);
    customPatterns->setFont(m_font);
    m_patternsTab->addTab(customPatterns, i18n("Custom Pattern"));

    connect(m_patternChooser, SIGNAL(resourceSelected(KoResourceSP )),
            view->canvasResourceProvider(), SLOT(slotPatternActivated(KoResourceSP )));

    connect(customPatterns, SIGNAL(activatedResource(KoResourceSP )),
            view->canvasResourceProvider(), SLOT(slotPatternActivated(KoResourceSP )));

    connect(view->canvasResourceProvider(), SIGNAL(sigPatternChanged(KoPatternSP)),
            this, SLOT(slotSetPattern(KoPatternSP)));

    m_patternChooser->setCurrentItem(0);
    if (m_patternChooser->currentResource() && view->canvasResourceProvider()) {
        view->canvasResourceProvider()->slotPatternActivated(m_patternChooser->currentResource());
    }

    m_patternWidget->setPopupWidget(m_patternChooserPopup);


}

void KisControlFrame::createGradientsChooser(KisViewManager * view)
{
    if (m_gradientChooserPopup) {
        delete m_gradientChooserPopup;
        m_gradientChooserPopup = 0;
    }

    m_gradientChooserPopup = new QWidget(m_gradientWidget);
    m_gradientChooserPopup->setObjectName("gradient_chooser_popup");
    QHBoxLayout * l2 = new QHBoxLayout(m_gradientChooserPopup);
    l2->setObjectName("gradientpopuplayout");

    m_gradientTab = new QTabWidget(m_gradientChooserPopup);
    m_gradientTab->setObjectName("gradientstab");
    m_gradientTab->setFocusPolicy(Qt::NoFocus);
    m_gradientTab->setFont(m_font);
    l2->addWidget(m_gradientTab);

    m_gradientChooser = new KisGradientChooser(m_gradientChooserPopup);
    m_gradientChooser->setCanvasResourcesInterface(view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    m_gradientChooser->setFont(m_font);
    QWidget *gradientChooserPage = new QWidget(m_gradientChooserPopup);
    QHBoxLayout *gradientChooserPageLayout  = new QHBoxLayout(gradientChooserPage);
    gradientChooserPageLayout->addWidget(m_gradientChooser);
    m_gradientTab->addTab(gradientChooserPage, i18n("Gradients"));

    connect(m_gradientChooser, SIGNAL(resourceSelected(KoResourceSP)),
            view->canvasResourceProvider(), SLOT(slotGradientActivated(KoResourceSP)));
    connect (view->mainWindow(), SIGNAL(themeChanged()), m_gradientChooser, SLOT(slotUpdateIcons()));
    connect(view->canvasResourceProvider(), SIGNAL(sigGradientChanged(KoAbstractGradientSP)),
            this, SLOT(slotSetGradient(KoAbstractGradientSP)));

    m_gradientChooser->setCurrentItem(0);

    if (m_gradientChooser->currentResource() && view->canvasResourceProvider())
        view->canvasResourceProvider()->slotGradientActivated(m_gradientChooser->currentResource());

    m_gradientWidget->setPopupWidget(m_gradientChooserPopup);

}


