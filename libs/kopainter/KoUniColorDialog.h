/**
 * Copyright (c) 2006 Casper Boemann (cbr@boemann.dk)
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
#ifndef KOUNICOLORDIALOG_H
#define KOUNICOLORDIALOG_H

#include "KoColor.h"
#include "KoUniColorChooser.h"

#include <kpagedialog.h>
#include <koffice_export.h>


/**
 * @short A colormanaged dialog for selecting a colors.
 *
 * KoUniColorDialog is simply the dialog'ification of KoUniColorChooser.
 *
 */
class KOPAINTER_EXPORT KoUniColorDialog
     : public KPageDialog
{
    Q_OBJECT
    typedef KPageDialog super;

public:
    KoUniColorDialog(KoColor &initialColor, QWidget *parent = 0L);
    virtual ~KoUniColorDialog() {}
};

#endif
