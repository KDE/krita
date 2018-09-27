/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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
#include <kritaui_export.h>

class KoID;
class KoColorSpace;
class KisAdvancedColorSpaceSelector;

class KRITAUI_EXPORT KisColorSpaceSelector : public QWidget
{
    Q_OBJECT
public:
    KisColorSpaceSelector(QWidget* parent);
    ~KisColorSpaceSelector() override;
    const KoColorSpace* currentColorSpace();
    void setCurrentColorModel(const KoID& id);
    void setCurrentColorDepth(const KoID& id);
    void setCurrentProfile(const QString& name);
    void setCurrentColorSpace(const KoColorSpace* colorSpace);
    void showColorBrowserButton(bool showButton);
Q_SIGNALS:
    /**
     * This signal is emitted when a new color space is selected.
     * @param valid indicates if the color space can be used
     */
    void selectionChanged(bool valid);
    /// This signal is emitted, when a new color space is selected, that can be used (eg is valid)
    void colorSpaceChanged(const KoColorSpace*);
private Q_SLOTS:
    void fillCmbDepths(const KoID& idd);
    void fillCmbProfiles();
    void colorSpaceChanged();
    void installProfile();
    void slotOpenAdvancedSelector();
    void slotProfileValid(bool valid);
private:
    struct Private;
    KisAdvancedColorSpaceSelector *m_advancedSelector;
    Private * const d;

};


#endif
