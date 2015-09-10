/* This file is part of the KDE project

   Copyright 2012 Brijesh Patel <brijesh3105@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "FontEditorDialog.h"

#include <klocalizedstring.h>
#include <kfontchooser.h>

using namespace KoChart;


FontEditorDialog::FontEditorDialog(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Select Font"));
    setModal(true);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    QStringList list;
    KFontChooser::getFontList(list, KFontChooser::SmoothScalableFonts);
    fontChooser = new KFontChooser(this, KFontChooser::NoDisplayFlags, list, 7);

    setMainWidget(fontChooser);
}

FontEditorDialog::~FontEditorDialog()
{
}
