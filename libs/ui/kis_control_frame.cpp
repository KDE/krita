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
#include <QWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QWidgetAction>

#include <klocalizedstring.h>
#include <QAction>
#include <kactioncollection.h>

#include <KoDualColorButton.h>
#include <KoForegroundColour.h>
#include <KoBackgroundColour.h>
#include <KoSwapBgFgColours.h>
#include <KoResetBgFgColours.h>

#include <resources/KoAbstractGradient.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoColorSpaceRegistry.h>
#include <kis_image.h>

#include <resources/KoPattern.h>
#include "KisResourceServerProvider.h"
#include "kis_canvas_resource_provider.h"

#include "widgets/kis_iconwidget.h"

#include "widgets/gradient/KisGradientChooser.h"
#include "KisViewManager.h"
#include "kis_config.h"
#include "kis_paintop_box.h"
#include "KisDockerHud.h"
#include "kis_custom_pattern.h"
#include "widgets/kis_pattern_chooser.h"
#include "kis_favorite_resource_manager.h"
#include "kis_display_color_converter.h"
#include <kis_canvas2.h>
#include "kis_action_registry.h"


KisControlFrame::KisControlFrame(KisViewManager *view, QWidget *parent, const char* name)
    : QObject(view)
    , m_viewManager(view)
    , m_checkersPainter(4)
{
    setObjectName(name);

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
    connect(action, SIGNAL(triggered()), m_patternWidget, SLOT(showPopupWidget()));
    m_patternChooserPopup->addAction(action);

    action = new QWidgetAction(this);
    action->setText(i18n("&Gradients"));
    m_viewManager->actionCollection()->addAction(ResourceType::Gradients, action);
    action->setDefaultWidget(m_gradientWidget);
    connect(action, SIGNAL(triggered()), m_gradientWidget, SLOT(showPopupWidget()));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_gradientWidget, SLOT(update()));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_gradientWidget, SLOT(update()));
    m_gradientChooserPopup->addAction(action);


    // XXX: KOMVC we don't have a canvas here yet, needs a setImageView
    const KoColorDisplayRendererInterface *displayRenderer = \
        KisDisplayColorConverter::dumbConverterInstance()->displayRendererInterface();
    m_dual = new KoDualColorButton(m_viewManager->canvasResourceProvider(), displayRenderer,
                                                     m_viewManager->mainWindowAsQWidget(), m_viewManager->mainWindowAsQWidget()); 
    m_dual->setPopDialog(true);
    action = new QWidgetAction(this);
    action->setText(i18n("&Choose foreground and background colors"));
    action->setIcon(KisIconUtils::loadIcon("dual_colour"));
    m_viewManager->actionCollection()->addAction("dual", action);
    action->setDefaultWidget(m_dual);
    connect(m_dual, SIGNAL(foregroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(m_dual, SIGNAL(backgroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetBGColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_dual, SLOT(setBackgroundColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_dual, SLOT(setForegroundColor(KoColor)));
    m_dual->setFixedSize(28, 28);
    connect(m_viewManager, &KisViewManager::viewChanged, [this]() {
        slotUpdateDisplayRenderer(m_dual);
    });


    m_foreground = new KoForegroundColour(m_viewManager->canvasResourceProvider(), displayRenderer,
                                                    m_viewManager->mainWindowAsQWidget(), m_viewManager->mainWindowAsQWidget());
    m_foreground->setPopDialog(true);
    action = new QWidgetAction(this);
    action->setText(i18n("&Choose foreground colour"));
    action->setIcon(KisIconUtils::loadIcon("forgound_colour"));
    m_viewManager->actionCollection()->addAction("chooseForegroundColor", action);
    action->setDefaultWidget(m_foreground);
    connect(m_foreground, SIGNAL(foregroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(m_foreground, SIGNAL(backgroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetBGColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_foreground, SLOT(setBackgroundColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_foreground, SLOT(setForegroundColor(KoColor)));
    m_foreground->setFixedSize(28, 28);
    connect(action, SIGNAL(triggered()), m_foreground, SLOT(openForegroundDialog()));
    connect(m_viewManager, &KisViewManager::viewChanged, [this](){
        slotUpdateDisplayRenderer(m_foreground);
    });


    m_background = new KoBackgroundColour(m_viewManager->canvasResourceProvider(), displayRenderer,
                                                    m_viewManager->mainWindowAsQWidget(), m_viewManager->mainWindowAsQWidget());
    m_background->setPopDialog(true);
    action = new QWidgetAction(this);
    action->setText(i18n("&Choose background colour"));
    action->setIcon(KisIconUtils::loadIcon("background_colour"));
    m_viewManager->actionCollection()->addAction("chooseBackgroundColor", action);
    action->setDefaultWidget(m_background);
    connect(m_background, SIGNAL(foregroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(m_background, SIGNAL(backgroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetBGColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_background, SLOT(setBackgroundColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_background, SLOT(setForegroundColor(KoColor)));
    m_background->setFixedSize(28, 28);
    connect(action, SIGNAL(triggered()), m_background, SLOT(openBackgroundDialog()));
    connect(m_viewManager, &KisViewManager::viewChanged, [this]() {
        slotUpdateDisplayRenderer(m_background);
    });


    m_swap = new KoSwapBgFgColours(m_viewManager->canvasResourceProvider(), displayRenderer,
                                                    m_viewManager->mainWindowAsQWidget(), m_viewManager->mainWindowAsQWidget());
    m_swap->setPopDialog(true);
    action = new QWidgetAction(this);
    action->setText(i18n("&Swap Foreground and Background Colors"));
    action->setIcon(KisIconUtils::loadIcon("swap_colour"));
    m_viewManager->actionCollection()->addAction("toggle_fg_bg", action);
    action->setDefaultWidget(m_swap);
    connect(m_swap, SIGNAL(foregroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(m_swap, SIGNAL(backgroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetBGColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigBGColorChanged(KoColor)), m_swap, SLOT(setBackgroundColor(KoColor)));
    connect(m_viewManager->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), m_swap, SLOT(setForegroundColor(KoColor)));
    m_swap->setFixedSize(28, 28);
    connect(action, SIGNAL(triggered()), m_swap, SLOT(swapColours()));
    connect(m_viewManager, &KisViewManager::viewChanged, [this]() {
        slotUpdateDisplayRenderer(m_swap);
    });


    m_reset = new KoResetBgFgColours(m_viewManager->canvasResourceProvider(), displayRenderer,
                                                    m_viewManager->mainWindowAsQWidget(), m_viewManager->mainWindowAsQWidget());
    m_reset->setPopDialog(true);
    action = new QWidgetAction(this);
    action->setText(i18n("&reset Foreground and Background Colors"));
    action->setIcon(KisIconUtils::loadIcon("reset_colour"));
    m_viewManager->actionCollection()->addAction("reset_fg_bg", action);
    action->setDefaultWidget(m_reset);
    connect(m_reset, SIGNAL(foregroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetFGColor(KoColor)));
    connect(m_reset, SIGNAL(backgroundColorChanged(KoColor)), m_viewManager->canvasResourceProvider(), SLOT(slotSetBGColor(KoColor)));
    m_reset->setFixedSize(28, 28);
    connect(action, SIGNAL(triggered()), m_reset, SLOT(ResetColours()));
    connect(m_viewManager, &KisViewManager::viewChanged, [this]() {
        slotUpdateDisplayRenderer(m_reset);
    });
    

    m_paintopBox = new KisPaintopBox(m_viewManager, parent, "paintopbox");

    action = new QWidgetAction(this);
    action->setText(i18n("&Painter's Tools"));
    m_viewManager->actionCollection()->addAction("paintops", action);
    action->setDefaultWidget(m_paintopBox);


    createDockerBox(m_viewManager);

    action = new QWidgetAction(this);
    action->setText(i18n("&Docker Box"));
    m_viewManager->actionCollection()->addAction("dockerBox", action);
    action->setDefaultWidget(m_dockerPopupButton);
}

void KisControlFrame::slotUpdateDisplayRenderer(KoDualColorButton* colourSelector)
{
    KoColorDisplayRendererInterface *displayRenderer = nullptr;

    if (m_viewManager->canvasBase()) {
        displayRenderer = m_viewManager->canvasBase()->displayColorConverter()->displayRendererInterface();
    } else {
        displayRenderer = KoDumbColorDisplayRenderer::instance();
    }

    colourSelector->setDisplayRenderer(displayRenderer);
}


void KisControlFrame::slotSetPattern(KoPatternSP pattern)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(pattern);
    m_patternWidget->setThumbnail(pattern->image());
    m_patternChooser->setCurrentPattern(pattern);
}
void KisControlFrame::slotSetGradient(KoAbstractGradientSP gradient)
{
    m_gradientChooser->setCurrentResource(gradient);
    updateGradientPreviewOnPopupButton(gradient);
}

void KisControlFrame::updateGradientPreviewOnPopupButton(KoAbstractGradientSP gradient)
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
    m_patternChooserPopup->setMinimumSize(450, 400);
    m_patternChooserPopup->setObjectName("pattern_chooser_popup");
    QHBoxLayout * l2 = new QHBoxLayout(m_patternChooserPopup);
    l2->setObjectName("patternpopuplayout");

    m_patternsTab = new QTabWidget(m_patternChooserPopup);
    m_patternsTab->setObjectName("patternstab");
    m_patternsTab->setFocusPolicy(Qt::NoFocus);
    l2->addWidget(m_patternsTab);

    m_patternChooser = new KisPatternChooser(m_patternChooserPopup);
    m_patternChooser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QWidget *patternChooserPage = new QWidget(m_patternChooserPopup);
    QHBoxLayout *patternChooserPageLayout  = new QHBoxLayout(patternChooserPage);
    patternChooserPageLayout->addWidget(m_patternChooser);
    m_patternsTab->addTab(patternChooserPage, i18n("Patterns"));

    KisCustomPattern *customPatterns = new KisCustomPattern(0, "custompatterns",
                                                            i18n("Custom Pattern"), m_viewManager);
    m_patternsTab->addTab(customPatterns, i18n("Custom Pattern"));

    connect(m_patternChooser, SIGNAL(resourceSelected(KoResourceSP )),
            view->canvasResourceProvider(), SLOT(slotPatternActivated(KoResourceSP )));

    connect(customPatterns, SIGNAL(activatedResource(KoResourceSP )),
            view->canvasResourceProvider(), SLOT(slotPatternActivated(KoResourceSP )));

    connect(customPatterns, SIGNAL(patternAdded(KoResourceSP)), m_patternChooser, SLOT(setCurrentPattern(KoResourceSP)));
    connect(customPatterns, SIGNAL(patternUpdated(KoResourceSP)), m_patternChooser, SLOT(setCurrentPattern(KoResourceSP)));

    connect(view->canvasResourceProvider(),
            &KisCanvasResourceProvider::sigPatternChanged,
            this,
            &KisControlFrame::slotSetPattern);
    
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
    l2->addWidget(m_gradientTab);

    m_gradientChooser = new KisGradientChooser(m_gradientChooserPopup);
    m_gradientChooser->setCanvasResourcesInterface(view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface());
    QWidget *gradientChooserPage = new QWidget(m_gradientChooserPopup);
    QHBoxLayout *gradientChooserPageLayout  = new QHBoxLayout(gradientChooserPage);
    gradientChooserPageLayout->addWidget(m_gradientChooser);
    m_gradientTab->addTab(gradientChooserPage, i18n("Gradients"));

    connect(m_gradientChooser, SIGNAL(resourceSelected(KoResourceSP)),
            view->canvasResourceProvider(), SLOT(slotGradientActivated(KoResourceSP)));
    connect (view->mainWindowAsQWidget(), SIGNAL(themeChanged()), m_gradientChooser, SLOT(slotUpdateIcons()));
    connect(view->canvasResourceProvider(), SIGNAL(sigGradientChanged(KoAbstractGradientSP)),
            this, SLOT(slotSetGradient(KoAbstractGradientSP)));
    connect(m_gradientChooser, SIGNAL(gradientEdited(KoAbstractGradientSP)),
            this, SLOT(updateGradientPreviewOnPopupButton(KoAbstractGradientSP)));


    // set the Foreground to Transparent gradient as default on startup
    KisResourceModel resModel(ResourceType::Gradients);
    QVector<KoResourceSP> resources = resModel.resourcesForFilename("Foreground to Transparent.svg");

    if (resources.size() > 0) {
        m_gradientChooser->setCurrentResource(resources.first());
        if (view->canvasResourceProvider()) {
            view->canvasResourceProvider()->slotGradientActivated(resources.first());
        }
    }

    m_gradientWidget->setPopupWidget(m_gradientChooserPopup);
}


void KisControlFrame::createDockerBox(KisViewManager * view)
{
    KConfigGroup grp =  KSharedConfig::openConfig()->group("krita").group("Toolbar BrushesAndStuff");
    int iconsize = grp.readEntry("IconSize", 22);
    // NOTE: buttonsize should be the same value as the one used in ktoolbar for all QToolButton
    int buttonsize = grp.readEntry("ButtonSize", 32);

    m_dockerPopupButton = new KisIconWidget();
    m_dockerPopupButton->setIcon(KisIconUtils::loadIcon("view-list-details"));
    m_dockerPopupButton->setToolTip(i18n("Docker box"));
    m_dockerPopupButton->setFixedSize(buttonsize, buttonsize);
    m_dockerPopupButton->setIconSize(QSize(iconsize, iconsize));
    m_dockerPopupButton->setAutoRaise(true);
    m_dockerPopupButton->setArrowVisible(false);

    m_dockerPopup = new KisDockerHud(i18n("Toolbar Docker Box"), "toolbar");
    // Set a reasonable minimum size so that dockers capable of being tiny are not,
    // but are not too big either
    m_dockerPopup->setMinimumHeight(300);
    m_dockerPopup->setMinimumWidth(300);
    m_dockerPopupButton->setPopupWidget(m_dockerPopup);

    QWidgetAction* action = new QWidgetAction(this);
    KisActionRegistry::instance()->propertizeAction("docker_box", action);
    view->actionCollection()->addAction("docker_box", action);
    connect(action, SIGNAL(triggered()), m_dockerPopupButton, SLOT(showPopupWidget()));
    m_dockerPopup->addAction(action);
}
