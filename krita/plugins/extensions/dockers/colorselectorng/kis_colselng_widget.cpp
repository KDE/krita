/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_colselng_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "kis_colselng_my_paint_shade_selector.h"

#include "kis_colselng_bar.h"
#include "kis_colselng_my_paint_shade_selector.h"
#include "kis_colselng_color_selector.h"
#include "kis_colselng_color_patches.h"
#include "kis_colselng_common_colors.h"

KisColSelNgWidget::KisColSelNgWidget(QWidget *parent) :
    QWidget(parent)
{
    setMinimumHeight(50);
    QWidget* bar = new KisColSelNgBar(this);
    QWidget* colorSelector = new KisColSelNgColorSelector(this);
    QWidget* myPaintShadeSel = new KisColSelNgMyPaintShadeSelector(this);
    QWidget* colorPatches = new KisColSelNgColorPatches(this);
    QWidget* commonColors = new KisColSelNgCommonColors(this);

    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(bar);
    verticalLayout->addWidget(colorSelector);
    verticalLayout->addWidget(myPaintShadeSel);

    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addWidget(colorPatches);

    QVBoxLayout* overallLayout = new QVBoxLayout(this);
    overallLayout->addLayout(horizontalLayout);
    overallLayout->addWidget(commonColors);
}

