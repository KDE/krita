/* This file is part of the KDE project
   Copyright (C) 2005 Johannes Schaub <litb_devel@web.de>
   Copyright (C) 2011 Arjen Hiemstra <ahiemstra@heimr.nl>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoZoomMode.h"
#include <klocalizedstring.h>

const char* const KoZoomMode::modes[] =
{
    I18N_NOOP("%1%"),
    I18N_NOOP("Fit Page Width"),
    I18N_NOOP("Fit Page"),
    0,
    I18N_NOOP("Actual Pixels"),
    0,
    0,
    0,
    I18N_NOOP("Fit Text Width")
};

qreal KoZoomMode::minimumZoomValue = 0.2;
qreal KoZoomMode::maximumZoomValue = 5.0;

QString KoZoomMode::toString(Mode mode)
{
    return i18n(modes[mode]);
}

KoZoomMode::Mode KoZoomMode::toMode(const QString& mode)
{
    if(mode == i18n(modes[ZOOM_WIDTH]))
        return ZOOM_WIDTH;
    else
    if(mode == i18n(modes[ZOOM_PAGE]))
        return ZOOM_PAGE;
    else
     if(mode == i18n(modes[ZOOM_PIXELS]))
        return ZOOM_PIXELS;
    else
     if(mode == i18n(modes[ZOOM_TEXT]))
        return ZOOM_TEXT;
    else
       return ZOOM_CONSTANT;
    // we return ZOOM_CONSTANT else because then we can pass '10%' or '15%'
    // or whatever, it's automatically converted. ZOOM_CONSTANT is
    // changeable, whereas all other zoom modes (non-constants) are normal
    // text like "Fit to xxx". they let the view grow/shrink according
    // to windowsize, hence the term 'non-constant'
}

qreal KoZoomMode::minimumZoom()
{
    return minimumZoomValue;
}

qreal KoZoomMode::maximumZoom()
{
    return maximumZoomValue;
}

qreal KoZoomMode::clampZoom(qreal zoom)
{
    return qMin(maximumZoomValue, qMax(minimumZoomValue, zoom));
}

void KoZoomMode::setMinimumZoom(qreal zoom)
{
    Q_ASSERT(zoom > 0.0f);
    minimumZoomValue = zoom;
}

void KoZoomMode::setMaximumZoom(qreal zoom)
{
    Q_ASSERT(zoom > 0.0f);
    maximumZoomValue = zoom;
}
