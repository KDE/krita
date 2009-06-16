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
#ifndef KOUNICOLORDIALOG_H
#define KOUNICOLORDIALOG_H

#include "KoColor.h"

#include <kpagedialog.h>
#include "kowidgets_export.h"

class KoUniColorDialogPrivate;

/**
 * @short A colormanaged dialog for selecting a colors.
 *
 * KoUniColorDialog is simply the dialog'ification of KoUniColorChooser.
 */
class KOWIDGETS_EXPORT KoUniColorDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit KoUniColorDialog(KoColor &initialColor, QWidget *parent = 0);
    virtual ~KoUniColorDialog();

    /**
      * @return the selected color
      */
    KoColor color() const;

private:
    KoUniColorDialogPrivate *d;
};

#endif
