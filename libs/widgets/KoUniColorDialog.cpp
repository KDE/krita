/**
 * Copyright (c) 2006 Casper Boemann (cbr@boemann.dk)
 * Copyright (c) 2009 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include <klocale.h>
#include "KoUniColorDialog.h"
#include "KoUniColorChooser.h"

class KoUniColorDialogPrivate
{
public:
    KoUniColorDialogPrivate() : chooser(0) {}
    KoUniColorChooser *chooser;
};

KoUniColorDialog::KoUniColorDialog(KoColor &initialColor, QWidget *parent)
    : KPageDialog(parent),
    d(new KoUniColorDialogPrivate())
{
    setFaceType(Plain);

    d->chooser = new KoUniColorChooser();
    d->chooser->setColor(initialColor);
    addPage(d->chooser, i18n("Color Selector"));
}

KoUniColorDialog::~KoUniColorDialog()
{
   delete d;
}

KoColor KoUniColorDialog::color() const
{
    return d->chooser->color();
}

#include <KoUniColorDialog.moc>
