/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOFONTREGISTERY_H
#define KOFONTREGISTERY_H

#include <QScopedPointer>
#include <QVector>

#include <KoFontLibraryResourceUtils.h>
#include "kritaflake_export.h"

/**
 * @brief The KoFontRegistery class
 * A wrapper around a freetype library.
 *
 * This class abstract away loading freetype faces from css
 * values. Internally it uses fontconfig to get the filename
 * and then loads the freetype face within a mutex-lock.
 *
 * It also provides a configuration function to handle all the
 * size and variation axis values.
 */
class KRITAFLAKE_EXPORT KoFontRegistery
{
public:
    KoFontRegistery();

    static KoFontRegistery *instance();

    /**
     * @brief facesForCSSValues
     * This selects a font with fontconfig using the given
     * values. If "text" is not empty, it will try to select
     * fallback fonts as well.
     *
     * @returns a vector of loaded FT_Faces, the "lengths" vector
     * will be filled with the lengths of consequetive characters
     * a face can be set on.
     */
    std::vector<FT_FaceUP> facesForCSSValues(QStringList families,
                                             QVector<int> &lengths,
                                             QString text = "",
                                             qreal size = -1,
                                             int weight = 400,
                                             int width = 100,
                                             bool italic = false,
                                             int slant = 0,
                                             QString language = QString());

    /**
     * @brief configureFaces
     * This configures a list of faces with pointSize and
     * variation settings.
     *
     * Sometimes setting the size can get quite tricky (like with
     * bitmap fonts), so this convenience function handles all that.
     *
     * @returns whether the configuration was successful.
     */
    bool configureFaces(const std::vector<FT_FaceUP> &faces,
                        qreal size,
                        qreal fontSizeAdjust,
                        int xRes,
                        int yRes,
                        QMap<QString, qreal> axisSettings);

private:
    class Private;

    friend class TestSvgText;

    /**
     * @brief addFontFilePathToRegistery
     * This adds a font file to the registery. Right now only used by unittests.
     *
     * @param path the path of the font file.
     * @return Whether adding the font file was succesful.
     */
    bool addFontFilePathToRegistery(QString path);

    /**
     * @brief addFontFileDirectoryToRegistery
     * This adds a directory of font files to the registery. Right now only used by unittests.
     * @param path the path of the directory.
     * @return whether it was succesful.
     */
    bool addFontFileDirectoryToRegistery(QString path);

    QScopedPointer<Private> d;
};

#endif // KOFONTREGISTERY_H
