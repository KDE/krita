/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#ifndef _KIS_ADVANCED_COLOR_SPACE_SELECTOR_H_
#define _KIS_ADVANCED_COLOR_SPACE_SELECTOR_H_

#include <QWidget>
#include <QDialog>
#include "kritaui_export.h"

#include "ui_wdgcolorspaceselectoradvanced.h"

class KoID;
class KoColorSpace;

/* Use KisColorSpaceSelector instead of directly using this one*/

class KRITAUI_EXPORT KisAdvancedColorSpaceSelector : public QDialog
{
    Q_OBJECT
public:
    KisAdvancedColorSpaceSelector(QWidget* parent, const QString &caption);
    ~KisAdvancedColorSpaceSelector() override;
    const KoColorSpace* currentColorSpace();
    void setCurrentColorModel(const KoID& id);
    void setCurrentColorDepth(const KoID& id);
    void setCurrentProfile(const QString& name);
    void setCurrentColorSpace(const KoColorSpace* colorSpace);
Q_SIGNALS:
    void selectionChanged(bool valid);
    void colorSpaceChanged(const KoColorSpace*);
private Q_SLOTS:
    void fillCmbDepths(const KoID& idd);
    void fillLstProfiles();
    void fillDescription();
    QString nameWhitePoint(QVector <double> whitePoint);
    void colorSpaceChanged();
    void installProfile();
private:
    struct Private;
    Private * const d;
};


#endif
