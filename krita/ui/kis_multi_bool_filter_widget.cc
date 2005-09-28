/*
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "kis_multi_bool_filter_widget.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>

KisBoolWidgetParam::KisBoolWidgetParam(  bool ninitvalue, QString nname) :
    initvalue(ninitvalue),
    name(nname)
{

}

KisMultiBoolFilterWidget::KisMultiBoolFilterWidget(QWidget * parent, const char * name, const char * caption, vKisBoolWidgetParam iwparam) : 
    KisFilterConfigWidget( parent, name )
{
    Q_INT32 m_nbboolWidgets = iwparam.size();

    this->setCaption(caption);

    QGridLayout *widgetLayout = new QGridLayout(this, m_nbboolWidgets + 1, 3);
    widgetLayout -> setColStretch ( 1, 1 );

    m_boolWidgets = new QCheckBox*[ m_nbboolWidgets ];

    for( Q_INT32 i = 0; i < m_nbboolWidgets; ++i)
    {
        m_boolWidgets[i] = new QCheckBox( this, iwparam[i].name.ascii());
        m_boolWidgets[i] -> setChecked( iwparam[i].initvalue );

        connect(m_boolWidgets[i], SIGNAL(toggled( bool ) ), SIGNAL(sigPleaseUpdatePreview()));

        QLabel* lbl = new QLabel(iwparam[i].name+":", this);
        widgetLayout -> addWidget( lbl, i , 0);

        widgetLayout -> addWidget( m_boolWidgets[i], i , 1);
    }
    QSpacerItem * sp = new QSpacerItem(1, 1);
    widgetLayout -> addItem(sp, m_nbboolWidgets, 0);

}

#include "kis_multi_bool_filter_widget.moc"
