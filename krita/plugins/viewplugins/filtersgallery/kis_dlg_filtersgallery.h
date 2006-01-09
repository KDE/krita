/*
 * This file is part of Krita
 *
 * Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KISDLGFILTERSPREVIEW_H
#define KISDLGFILTERSPREVIEW_H

#include <kdialogbase.h>

class KisView;
class KisFilter;
class QIconViewItem;
class QLabel;
class QHBoxLayout;
class KisPreviewWidget;
class KisWdgFiltersGallery;
class KisFiltersListView;
    
namespace Krita {
namespace Plugins {
namespace FiltersGallery {

/**
@author Cyrille Berger
*/
class KisDlgFiltersGallery : public KDialogBase
{
    Q_OBJECT
    public:
        KisDlgFiltersGallery(KisView* view, QWidget* parent,const char *name = "");
        ~KisDlgFiltersGallery();
    public:
        inline KisFilter* currentFilter() { return m_currentFilter; };
        inline  QWidget* currentConfigWidget() { return m_currentConfigWidget; }
    private slots:
        void slotConfigChanged();
        void refreshPreview();
        void selectionHasChanged ( QIconViewItem * item );
    private:
        KisWdgFiltersGallery* m_widget;
        KisView* m_view;
        QWidget* m_currentConfigWidget;
        KisFilter* m_currentFilter;
        QLabel* m_labelNoCW;
};

}
}
}

#endif
