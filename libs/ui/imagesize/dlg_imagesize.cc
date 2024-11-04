/*
 *  dlg_imagesize.cc - part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2009 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_imagesize.h"

#include "wdg_imagesize.h"
#include <klocalizedstring.h>

DlgImageSize::DlgImageSize(QWidget *parent, int width, int height, double resolution)
    : KoDialog(parent)
{
    setCaption(i18n("Scale To New Size"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page.reset(new WdgImageSize(this, width, height, resolution));
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page.data());
}

DlgImageSize::~DlgImageSize()
{
}

qint32 DlgImageSize::desiredWidth()
{
    return m_page->desiredWidth();
}

qint32 DlgImageSize::desiredHeight()
{
    return m_page->desiredHeight();
}

double DlgImageSize::desiredResolution()
{
    return m_page->desiredResolution();
}

KisFilterStrategy *DlgImageSize::filterType()
{
    return m_page->filterType();
}
