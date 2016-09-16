/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_colorize_dom_utils.h"

#include <KoColorSpaceRegistry.h>
#include "kis_dom_utils.h"
#include "lazybrush/kis_lazy_fill_tools.h"
#include "kis_kra_tags.h"
#include "kis_paint_device.h"

namespace KisDomUtils {
    void saveValue(QDomElement *parent, const QString &tag, const KisLazyFillTools::KeyStroke &stroke)
    {
        using namespace KRA;

        QDomDocument doc = parent->ownerDocument();
        QDomElement e = doc.createElement(tag);
        parent->appendChild(e);

        e.setAttribute("type", COLORIZE_KEYSTROKE);

        QString fileName = tag;
        fileName.replace("item", COLORIZE_KEYSTROKE);

        e.setAttribute(FILE_NAME, fileName);
        e.setAttribute(COLORIZE_KEYSTROKE_IS_TRANSPARENT, stroke.isTransparent);

        QByteArray colorData = QByteArray::fromRawData((const char*)stroke.color.data(), stroke.color.colorSpace()->pixelSize());
        e.setAttribute(COLORBYTEDATA, QString(colorData.toBase64()));
    }

    bool loadValue(const QDomElement &e, KisLazyFillTools::KeyStroke *stroke, const KoColorSpace *colorSpace)
    {
        using namespace KRA;

        if (!Private::checkType(e, COLORIZE_KEYSTROKE)) return false;

        stroke->isTransparent = toInt(e.attribute(COLORIZE_KEYSTROKE_IS_TRANSPARENT, "0"));

        QByteArray colorData = QByteArray::fromBase64(e.attribute(COLORBYTEDATA).toLatin1());
        KoColor color((const quint8*)colorData.data(), colorSpace);
        stroke->color = color;

        stroke->dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

        return true;
    }
}
