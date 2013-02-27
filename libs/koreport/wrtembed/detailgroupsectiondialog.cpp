/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "detailgroupsectiondialog.h"

#include <KLocalizedString>

/*
 *  Constructs a DetailGroupSectionDialog as a child of 'parent'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
DetailGroupSectionDialog::DetailGroupSectionDialog(QWidget* parent)
  : KDialog(parent)
{
    QWidget *widget = new QWidget(this);
    setupUi(widget);
    setMainWidget(widget);

    setButtons(Ok | Cancel);
    setCaption(i18n("Group Section Editor"));
}

/*
 *  Destroys the object and frees any allocated resources
 */
DetailGroupSectionDialog::~DetailGroupSectionDialog()
{
    // no need to delete child widgets, Qt does it all for us
}
