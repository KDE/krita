/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_COLOR_SPACE_SELECTOR_H_
#define _KIS_COLOR_SPACE_SELECTOR_H_

#include <QWidget>
#include <krita_export.h>

class KoID;
class KoColorSpace;

class KRITAUI_EXPORT KisColorSpaceSelector : public QWidget
{
    Q_OBJECT
public:
    KisColorSpaceSelector(QWidget* parent);
    ~KisColorSpaceSelector();
    const KoColorSpace* currentColorSpace();
    void setCurrentColorModel(const KoID& id);
    void setCurrentColorDepth(const KoID& id);
    void setCurrentProfile(const QString& name);
    void setCurrentColorSpace(const KoColorSpace* colorSpace);
signals:
    /**
     * This signal is emited when a new color space is selected.
     * @param valid indicates if the color space can be used
     */
    void selectionChanged(bool valid);
private slots:
    void fillCmbDepths(const KoID& idd);
    void fillCmbProfiles();
    void colorSpaceChanged();
private:
    struct Private;
    Private * const d;
};


#endif
