/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    bool loadValue(const QDomElement &e, KisLazyFillTools::KeyStroke *stroke, const KoColorSpace *colorSpace, const QPoint &offset)
    {
        using namespace KRA;

        if (!Private::checkType(e, COLORIZE_KEYSTROKE)) return false;

        stroke->isTransparent = toInt(e.attribute(COLORIZE_KEYSTROKE_IS_TRANSPARENT, "0"));

        QByteArray colorData = QByteArray::fromBase64(e.attribute(COLORBYTEDATA).toLatin1());
        KoColor color((const quint8*)colorData.data(), colorSpace);
        stroke->color = color;

        stroke->dev = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
        stroke->dev->moveTo(offset);

        return true;
    }
}
