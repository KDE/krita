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

#include <kdialog.h>

#include "ui_kis_wdg_filtersgallery.h"

class Q3IconViewItem;
class QLabel;

class KisView;
class KisFilter;

class KisWdgFiltersGallery : public QWidget, public Ui::KisWdgFiltersGallery
{
    Q_OBJECT

    public:
        KisWdgFiltersGallery(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

namespace Krita {
namespace Plugins {
namespace FiltersGallery {

/**
@author Cyrille Berger
*/
class KisDlgFiltersGallery : public KDialog
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
        void selectionHasChanged ( Q3IconViewItem * item );
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
