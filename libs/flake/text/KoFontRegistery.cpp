/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontRegistery.h"

#include <QGlobalStatic>
#include <QMutex>
#include <QDebug>

#include FT_MULTIPLE_MASTERS_H
#include <fontconfig/fontconfig.h>

Q_GLOBAL_STATIC(KoFontRegistery, s_instance)

class KoFontRegistery::Private
{
public:
    FT_Library library = 0;
};

KoFontRegistery::KoFontRegistery() : d(new Private())
{

}

KoFontRegistery *KoFontRegistery::instance()
{
    if (!s_instance.exists()) {
        s_instance->init();
    }
    return s_instance;
}

QVector<FT_Face> KoFontRegistery::facesForCSSValues(QStringList families,
                                                    QVector<int> &lengths,
                                                    QString text,
                                                    qreal size,
                                                    int weight,
                                                    int width,
                                                    bool italic,
                                                    int slant,
                                                    QString language)
{
    Q_UNUSED(size)
    Q_UNUSED(language)
    int errorCode = 0;
    //FcObjectSet *objectSet = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_WIDTH, FC_WEIGHT, FC_SLANT, nullptr);
    FcCharSet *charSet = FcCharSetNew();
    FcPattern *p = FcPatternCreate();
    for (QString family: families) {
        const FcChar8 *vals = reinterpret_cast<FcChar8*>(family.toUtf8().data());
        FcPatternAddString(p, FC_FAMILY, vals);
    }
    if (italic == true) {
        FcPatternAddInteger(p, FC_SLANT, FC_SLANT_ITALIC);
    } else if(slant != 0) {
        FcPatternAddInteger(p, FC_SLANT, FC_SLANT_ITALIC);
    } else {
        FcPatternAddInteger(p, FC_SLANT, FC_SLANT_ROMAN);
    }
    FcPatternAddInteger(p, FC_WEIGHT, FcWeightFromOpenType(weight));
    FcPatternAddInteger(p, FC_WIDTH, width);
    FcPatternAddBool(p, FC_OUTLINE, true);

    FcResult result;
    FcChar8 *fileValue = 0;
    FcFontSet *fontSet = FcFontSort(FcConfigGetCurrent(), p, FcTrue, &charSet, &result);

    QStringList fontFileNames;
    lengths.clear();

    if (text.isEmpty()) {
        for (int j = 0; j < fontSet->nfont; j++) {
            QString fontFileName;
            if (FcPatternGetString(fontSet->fonts[j], FC_FILE, 0, &fileValue) == FcResultMatch) {
                fontFileName = QString(reinterpret_cast<char*>(fileValue));
                fontFileNames.append(fontFileName);
                lengths.append(0);
                break;
            }
        }
    } else {
        QString fontFileName;
        FcChar8 *fileValue = 0;
        FcCharSet *set = 0;
        QVector<int> familyValues(text.size());
        familyValues.fill(-1);

        for (int i=0; i<fontSet->nfont; i++) {
            if (FcPatternGetCharSet(fontSet->fonts[i], FC_CHARSET, 0, &set) == FcResultMatch) {
                for (int j = 0; j < text.size(); ++j) {
                    if (familyValues.at(j) == -1) {
                        QString unicode = text.at(j);
                        if (text.at(j).isHighSurrogate()) {
                            unicode += text.at(j+1);
                        }
                        if (text.at(j).isLowSurrogate()) {
                            familyValues[j] = familyValues[j-1];
                            continue;
                        }
                        if (FcCharSetHasChar(set, unicode.toUcs4().first())) {
                            familyValues[j] = i;
                        }
                    }
                }
                if (!familyValues.contains(-1)) {
                    break;
                }
            }
        }
        int length = 0;
        int startIndex = 0;
        int lastIndex = familyValues.at(0);
        if (FcPatternGetString(fontSet->fonts[lastIndex], FC_FILE, 0, &fileValue) == FcResultMatch) {
            fontFileName = QString(reinterpret_cast<char*>(fileValue));
        }
        for (int i = 0; i< familyValues.size(); i++) {
            if (lastIndex != familyValues.at(i)) {
                lengths.append(text.mid(startIndex, length).size());
                fontFileNames.append(fontFileName);
                startIndex = i;
                length = 0;
                lastIndex = familyValues.at(i);
                if (lastIndex != -1) {
                    if (FcPatternGetString(fontSet->fonts[lastIndex], FC_FILE, 0, &fileValue) == FcResultMatch) {
                        fontFileName = QString(reinterpret_cast<char*>(fileValue));
                    }
                }
            }
            length +=1;
        }
        if (length > 0) {
            lengths.append(text.mid(startIndex, length).size());
            fontFileNames.append(fontFileName);
        }
    }

    QVector<FT_Face> faces;
    QMutex mutex;

    for (int i = 0; i < lengths.size(); i++) {
        mutex.lock();
        FT_Face face = NULL;
        if (FT_New_Face(d->library, fontFileNames.at(i).toUtf8().data(), 0, &face) == 0) {
            faces.append(face);
        }
        mutex.unlock();
    }

    return faces;
}

bool KoFontRegistery::configureFaces(QVector<FT_Face> &faces,
                                     qreal size,
                                     int xRes,
                                     int yRes,
                                     QMap<QString, qreal> axisSettings)
{
    int errorCode = 0;
    int ftFontUnit = 64.0;
    qreal finalRes = qMin(xRes, yRes);
    qreal scaleToPixel = float(finalRes/72.);
    for (FT_Face face: faces) {
        if (!FT_IS_SCALABLE(face)) {
            int fontSizePixels = size * ftFontUnit * scaleToPixel;
            int sizeDelta = 0;
            int selectedIndex = -1;

            for (int i=0; i<face->num_fixed_sizes; i++) {
                int newDelta = qAbs((fontSizePixels) - face->available_sizes[i].x_ppem);
                if (newDelta < sizeDelta || i == 0) {
                    selectedIndex = i;
                    sizeDelta = newDelta;
                }
            }

            if (selectedIndex >= 0) {
                if (FT_HAS_COLOR(face)) {
                    long scale = long(65535 * qreal(fontSizePixels)/qreal(face->available_sizes[selectedIndex].x_ppem));
                    FT_Matrix matrix;
                    matrix.xx = scale;
                    matrix.xy = 0;
                    matrix.yx = 0;
                    matrix.yy = scale;
                    FT_Vector v;
                    FT_Set_Transform(face, &matrix, &v);
                }
                errorCode = FT_Select_Size(face, selectedIndex);
            }
        } else {
            errorCode = FT_Set_Char_Size(face, size * ftFontUnit, 0, xRes, yRes);
        }

        QMap<FT_Tag, qreal> tags;
        for (QString tagName: axisSettings.keys()) {
            if (tagName.size() == 4) {
                char *t = tagName.toUtf8().data();
                tags.insert(FT_MAKE_TAG(t[0], t[1], t[2], t[3]), axisSettings.value(tagName));
            }
        }
        if (FT_HAS_MULTIPLE_MASTERS(face)) {
            FT_MM_Var*  amaster = nullptr;
            FT_Get_MM_Var(face, &amaster);
            // note: this only works for opentype, as it uses tag-based-selection.
            FT_Fixed designCoords[amaster->num_axis];
            for (FT_UInt i = 0; i < amaster->num_axis; i++) {
                FT_Var_Axis axis = amaster->axis[i];
                for (FT_Tag tag: tags.keys()) {
                    if (axis.tag == tag) {
                        designCoords[i] = qBound(axis.minimum,
                                                long(tags.value(tag) * 65535),
                                                axis.maximum);
                    }
                }
            }
            FT_Set_Var_Design_Coordinates(face, amaster->num_axis, designCoords);
            FT_Done_MM_Var(d->library, amaster);
        }
    }
    return (errorCode == 0);
}

void KoFontRegistery::init()
{
    FT_Error error;
    error = FT_Init_FreeType( &d->library );
    if (error) {
        qWarning() << "Error with initializing FreeType library:" << error;
    }
}
