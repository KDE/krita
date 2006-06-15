/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef DLG_SEPARATE
#define DLG_SEPARATE

#include <kdialog.h>
#include <kis_channel_separator.h>

class WdgSeparations;

/**
 * This dialog allows the user to configure the decomposition of an image
 * into layers: one layer for each color channel.
 */
class DlgSeparate: public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:

    DlgSeparate(const QString & imageCS, const QString & layerCS, QWidget * parent = 0,
             const char* name = 0);
    ~DlgSeparate();

public:

    enumSepAlphaOptions getAlphaOptions();
    enumSepSource getSource();
    enumSepOutput getOutput();

    bool getDownscale();
    void enableDownscale(bool enable);

    bool getToColor();


private slots:
    
    void slotSetColorSpaceLabel(int buttonid);
    void okClicked();

private:

    WdgSeparations * m_page;
    QString m_imageCS;
    QString m_layerCS;

};

#endif // DLG_SEPARATE
