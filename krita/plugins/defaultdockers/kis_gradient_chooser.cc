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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <koIconChooser.h>

#include "kis_gradient_chooser.h"
#include "kis_global.h"
#include "kis_icon_item.h"
#include "kis_gradient.h"

KisGradientChooser::KisGradientChooser(QWidget *parent, const char *name) : super(parent, name)
{
	m_lbName = new QLabel(this);

	QVBoxLayout *mainLayout = new QVBoxLayout(this, 2, -1, "main layout");

	mainLayout -> addWidget(m_lbName);
	mainLayout -> addWidget(chooserWidget(), 10);
}

KisGradientChooser::~KisGradientChooser()
{
}

void KisGradientChooser::update(KoIconItem *item)
{
	KisIconItem *kisItem = static_cast<KisIconItem *>(item);

	if (item) {
		KisGradient *gradient = static_cast<KisGradient *>(kisItem -> resource());

		m_lbName -> setText(gradient -> name());
	}
}

#include "kis_gradient_chooser.moc"

