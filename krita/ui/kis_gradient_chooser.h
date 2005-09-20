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
#ifndef KIS_GRADIENT_CHOOSER_H_
#define KIS_GRADIENT_CHOOSER_H_

#include <kdialogbase.h>

#include "kis_itemchooser.h"

class QLabel;
class QPushButton;
class KisView;

class KisCustomGradientDialog : public KDialogBase {

    Q_OBJECT

public:

    KisCustomGradientDialog(KisView * view, QWidget * parent, const char *name);
    
private:
    
    KisAutogradient * m_page;

};

class KisGradientChooser : public KisItemChooser {
    typedef KisItemChooser super;
    Q_OBJECT

public:
    // XXX: On library redesign, remove m_view parameter here, it's just a temporary hack for the autogradient dialog!
    KisGradientChooser(KisView * view, QWidget *parent = 0, const char *name = 0);
    virtual ~KisGradientChooser();

protected:
    virtual void update(KoIconItem *item);

private:
    QLabel *m_lbName;
    QPushButton * m_customGradient;
};

#endif // KIS_GRADIENT_CHOOSER_H_

