/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_DOCKER_MANAGER_
#define KIS_DOCKER_MANAGER_

#include "qobject.h"
#include "qptrlist.h"

#include "kis_image.h"
#include "kis_selection.h"
#include <koffice_export.h>

#include "kis_generic_registry.h"
#include "kis_global.h"

class KAction;
class KisView;
class KisDockFrameDocker;
class KoTabbedToolDock;
class KoToolDockManager;
class QWidget;
class QToolBox;
class KisTool;
class KisPattern;
class KisBrush;
class KisGradient;
class KActionCollection;
class KisPaintBox;
class KisLayerBox;
class KisPaintBox;
class KisFilterBox;
class ControlFrame;
class KisBirdEyeBox;
class KisChannelView;
class KisAutobrush;
class KisTextBrush;
class KisAutogradient;
class KisHSVWidget;
class KisRGBWidget;
class KisGrayWidget;
class KisPaletteWidget;
class KisResourceMediator;


/**
 * The docker manager keeps track of dockers and their tabs.
 *
 * There are three kinds of dockers: sliding dockers, shading
 * dockers and toolboxes.
 */
class KRITACORE_EXPORT KisDockerManager : public QObject {

	Q_OBJECT;

public:

	KisDockerManager(KisView * view, KActionCollection * ac);
	virtual ~KisDockerManager();

	void addDockerTab(QWidget * tab, const QString & docker, enumDockerStyle docktype);

	void setToolOptionWidget(KisTool * oldTool, KisTool * newTool);
	void unsetToolOptionWidget(KisTool * oldTool);
	void resetLayerBox(KisImageSP img, KisLayerSP layer);
	
private:

	KisGenericRegistry<QWidget*> * m_tabs;
	KisGenericRegistry<KisDockFrameDocker*> * m_dockWindows;
	KisGenericRegistry<QToolBox*> * m_toolBoxes;
	KisGenericRegistry<KoTabbedToolDock*> * m_sliders;

	KisView * m_view;
	KActionCollection * m_ac;

	

// XXX: Temporary copies from KisView

private:

	void setupDockers();

private slots:

	void slotBrushChanged(KisBrush * brush);
	void slotGradientChanged(KisGradient * gradient);
	void slotPatternChanged(KisPattern * pattern);

 	void viewColorSlider(bool v = true);
 	void viewControlSlider(bool v = true);
 	void viewLayerChannelSlider(bool v = true);
 	void viewShapesSlider(bool v = true);
 	void viewFillsSlider(bool v = true);

	void viewColorDocker();
	void viewControlDocker();
	void viewLayerChannelDocker();
	void viewShapesDocker();
	void viewFillsDocker();
	void viewPaintBoxDocker();

	
private:
	
	KoToolDockManager * m_toolDockManager;

        // Sliders
	KoTabbedToolDock *m_layerchannelslider;
	KoTabbedToolDock *m_shapesslider;
	KoTabbedToolDock *m_fillsslider;
	KoTabbedToolDock *m_toolcontrolslider;
	KoTabbedToolDock *m_colorslider;

	// Dockers
	KisDockFrameDocker *m_layerchanneldocker;
	KisDockFrameDocker *m_shapesdocker;
	KisDockFrameDocker *m_fillsdocker;
	KisDockFrameDocker *m_toolcontroldocker;
	KisDockFrameDocker *m_colordocker;

	// Toolbox
	KisPaintBox *m_paintboxdocker;

	// Widgets
	
	KisLayerBox *m_layerBox;
	KisPaintBox *m_paintBox;
	KisFilterBox * m_filterBox;
	ControlFrame *m_controlWidget;
	KisBirdEyeBox * m_birdEyeBox;
	KisChannelView *m_channelView;
	
	KisAutobrush *m_autobrush;
	KisTextBrush *m_textBrush;
	KisAutogradient* m_autogradient;

	KisHSVWidget *m_hsvwidget;
	KisRGBWidget *m_rgbwidget;
	KisGrayWidget *m_graywidget;
	KisPaletteWidget *m_palettewidget;

	KisResourceMediator *m_brushMediator;
	KisResourceMediator *m_patternMediator;
	KisResourceMediator *m_gradientMediator;

	bool m_slidersSetup;

};

#endif // KIS_DOCKER_MANAGER_
