/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSDTEXTDATAPARSER_H
#define PSDTEXTDATAPARSER_H

#include <QScopedPointer>
#include <QStringList>
#include <QRect>
#include <QVariantHash>
#include <QTransform>
#include "kritapsdutils_export.h"

class KoSvgTextShape;
class KoColorSpace;
class KoShape;

/**
 * @brief The PsdTextDataConverter class
 *
 * This class handles converting PSD text engine data to actual SVG.
 */
class KRITAPSDUTILS_EXPORT PsdTextDataConverter
{
public:
    PsdTextDataConverter(KoSvgTextShape *shape);
    ~PsdTextDataConverter();

    bool convertPSDTextEngineDataToSVG(const QVariantHash tySh,
                                       const QVariantHash txt2,
                                       const KoColorSpace *imageCs,
                                       const int textIndex,
                                       QString *svgText,
                                       QString *svgStyles,
                                       QPointF &offset,
                                       bool &offsetByAscent,
                                       bool &isHorizontal,
                                       QTransform scaleToPt = QTransform());
    bool convertToPSDTextEngineData(const QString &svgText,
                                    QRectF &boundingBox,
                                    const QList<KoShape *> &shapesInside,
                                    QVariantHash &txt2,
                                    int &textIndex,
                                    QString &textTotal,
                                    bool &isHorizontal,
                                    QTransform scaleToPx = QTransform());

    /**
     * A list of errors happened during loading the user's text
     */
    QStringList errors() const;
    /**
     * A list of warnings produced during loading the user's text
     */
    QStringList warnings() const;
private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // PSDTEXTDATAPARSER_H
