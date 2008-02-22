/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_pattern_chooser.h"

#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>

#include <klocale.h>
#include <kfiledialog.h>
#include <KoResourceItemChooser.h>

#include "KoColorSpace.h"

#include "kis_global.h"
#include "kis_icon_item.h"
#include "kis_pattern.h"
#include "kis_resourceserverprovider.h"

KisPatternChooser::KisPatternChooser(QWidget *parent, const char *name) : KisItemChooser(parent, name)
{
    m_lbName = new QLabel(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->setSpacing(2);

    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(chooserWidget(), 10);

    connect( this, SIGNAL( importClicked() ), this, SLOT( slotImportPattern() ) );
}

KisPatternChooser::~KisPatternChooser()
{
}

void KisPatternChooser::update(QTableWidgetItem *item)
{
    KoResourceItem *kisItem = static_cast<KoResourceItem *>(item);

    if (item) {
        KisPattern *pattern = static_cast<KisPattern *>(kisItem->resource());

        QString text = QString("%1 (%2 x %3)").arg(pattern->name()).arg(pattern->width()).arg(pattern->height());

        m_lbName->setText(text);
    }
}

void KisPatternChooser::slotImportPattern()
{
    QString filter( "*.jpg *.gif *.png *.tif *.xpm *.bmp" );
    QString filename = KFileDialog::getOpenFileName( KUrl(), filter, 0, i18n( "Choose Pattern to Add" ) );

    KisResourceServerProvider::instance()->patternServer()->importResource(filename);
}

#include "kis_pattern_chooser.moc"

