/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFFWWSConverter.h"

#include <KisForest.h>
#include <hb.h>
#include <hb-ft.h>
#include FT_TRUETYPE_TABLES_H

#include <QFileInfo>

QString KoFontFamilyNode::debugInfo()
{
    QString style = isItalic? isOblique? "Oblique": "Italic": "Roman";
    QStringList axisInfo;
    for (int i=0; i< axes.size(); i++) {
        KoFontFamilyAxis axis = axes.value(axes.keys().at(i));
        axisInfo.append(QString(axis.tag+"="+QString::number(axis.value)));
    }
    QString debug = QString("\'%1\' \'%2\', style: %3, axes: %4").arg(fontFamily, fontStyle, style, axisInfo.join(", "));
    if (!pixelSizes.isEmpty()) {
        QStringList pix;
        for (int i=0; i< pixelSizes.size(); i++) {
            pix.append(QString::number(pixelSizes.keys().at(i)));
        }
        debug += " pixelSizes: "+pix.join(", ");
    }
    return debug;
}

struct KoFFWWSConverter::Private {
    Private() {}

    KisForest<KoFontFamilyNode> fontFamilyCollection;
};

KoFFWWSConverter::KoFFWWSConverter()
    : d(new Private())
{

}

KoFFWWSConverter::~KoFFWWSConverter()
{
}

/// @See https://learn.microsoft.com/en-us/typography/opentype/spec/os2#uswidthclass
qreal percentageFromUsWidthClass(int width) {
    switch(width) {
    case 1:
        return 50.0;
    case 2:
        return 62.5;
    case 3:
        return 75;
    case 4:
        return 87.5;
    case 5:
        return 100.0;
    case 6:
        return 112.5;
    case 7:
        return 125;
    case 8:
        return 150;
    case 9:
        return 200;
    default:
        return 100;
    }
}

constexpr unsigned ITALIC = 1u << 0;
constexpr unsigned BOLD = 1u << 5;
constexpr unsigned REGULAR = 1u << 6;
constexpr unsigned WWS = 1u << 8;
constexpr unsigned OBLIQUE = 1u << 9;
constexpr unsigned USE_TYPO_METRICS = 1u << 7;

bool KoFFWWSConverter::addFontFromPattern(const FcPattern *pattern, FT_LibrarySP freeTypeLibrary)
{

    KoFontFamilyNode fontFamily;
    if (!freeTypeLibrary.data()) {
        return false;
    }

    bool getFile = false;
    FcChar8 *fileValue{};
    if (FcPatternGetString(pattern, FC_FILE, 0, &fileValue) != FcResultMatch) {
        qWarning() << "Failed to get font file for" << pattern;
    } else {
        getFile = true;
    }
    fontFamily.fileName = QString::fromUtf8(reinterpret_cast<char *>(fileValue));

    int indexValue{};
    if (FcPatternGetInteger(pattern, FC_INDEX, 0, &indexValue) != FcResultMatch) {
        qWarning() << "Failed to get font index for" << pattern << "(file:" << fontFamily.fileName << ")";
        getFile = false;
    } else {
        fontFamily.fileIndex = indexValue;
    }

    if (getFile == false) {
        return getFile;
    }


    FT_Face f = nullptr;
    FT_FaceSP face;
    QByteArray utfData = fontFamily.fileName.toUtf8();
    if (FT_New_Face(freeTypeLibrary.data(), utfData.data(), fontFamily.fileIndex, &f) == 0) {
        face.reset(f);
    } else {
        return false;
    }

    fontFamily.fontFamily = face->family_name;
    fontFamily.fontStyle = face->style_name;
    KoFontFamilyNode typographicFamily;
    KoFontFamilyNode wwsFamily;
    bool isWWSFamilyWithoutName = false;

    if (!FT_IS_SFNT(face.data())) {
        fontFamily.isItalic = face->style_flags & FT_STYLE_FLAG_ITALIC;
        if (face->style_flags & FT_STYLE_FLAG_BOLD) {
            fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(700));
        } else {
            fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(400));
        }
        if (fontFamily.fontFamily.isEmpty()) {
            fontFamily.fontFamily = QFileInfo(fontFamily.fileName).baseName();
        }
        for (int i=0; i< face->num_fixed_sizes; i++) {
            fontFamily.pixelSizes.insert((face->available_sizes[i].size / 64.0), fontFamily.fileName);
        }
    } else {
        hb_face_t *hbFace = hb_ft_face_create_referenced(face.data());

        // Retrieve width, weight and slant data.
        TT_OS2 *os2Table = nullptr;
        os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
        if (os2Table) {
            fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(os2Table->usWeightClass));
            fontFamily.axes.insert("wdth", KoFontFamilyAxis::widthAxis(percentageFromUsWidthClass(os2Table->usWidthClass)));
            if (os2Table->fsSelection & BOLD && os2Table->usWeightClass == 400) {
                fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(700));
            }
            fontFamily.isItalic = (os2Table->fsSelection & ITALIC);
            fontFamily.isOblique = (os2Table->fsSelection & OBLIQUE);
            if (os2Table->fsSelection & REGULAR) {
                fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(400));
                fontFamily.isItalic = false;
                fontFamily.isOblique = false;
            }
            if (os2Table->version >= 5) {
                FontFamilySizeInfo sizeInfo;
                sizeInfo.high = os2Table->usUpperOpticalPointSize;
                sizeInfo.low = os2Table->usLowerOpticalPointSize;
                sizeInfo.os2table = true;
                fontFamily.sizeInfo = sizeInfo;
            }
        }

        // retrieve gpos size data...
        uint designSize;
        uint subFamilyId;
        uint rangeStart;
        uint rangeEnd;
        hb_ot_name_id_t sizeNameId;
        if (hb_ot_layout_get_size_params(hbFace, &designSize, &subFamilyId, &sizeNameId, &rangeStart, &rangeEnd)) {
            FontFamilySizeInfo sizeInfo;
            sizeInfo.low = rangeStart;
            sizeInfo.high = rangeEnd;
            sizeInfo.uniqueIDX = subFamilyId;
            sizeInfo.designSize = designSize;
            fontFamily.sizeInfo = sizeInfo;
        }

        // retrieve axis data...

        QHash<hb_ot_name_id_t, QString> axisNameIDs;
        if (hb_ot_var_has_data(hbFace)) {
            uint count = hb_ot_var_get_axis_count(hbFace);
            uint maxInfos = 1;
            for (FT_UInt i = 0; i < count; i++) {
                KoFontFamilyAxis axisInfo;
                hb_ot_var_axis_info_t axis;
                hb_ot_var_get_axis_infos(hbFace, i, &maxInfos, &axis);
                axisInfo.min = axis.min_value;
                axisInfo.max = axis.max_value;
                axisInfo.value = axis.default_value;
                char buff[4];
                hb_tag_to_string(axis.tag, buff);
                axisInfo.tag = QString::fromLatin1(buff, 4);
                axisNameIDs.insert(axis.name_id, axisInfo.tag);
                fontFamily.axes.insert(axisInfo.tag, axisInfo);
            }
        }

        uint numEntries = 0;
        const hb_ot_name_entry_t *entries = hb_ot_name_list_names(hbFace, &numEntries);

        QHash<QString, QString> ribbiFamilyNames;
        QHash<QString, QString> WWSFamilyNames;
        QHash<QString, QString> typographicFamilyNames;
        for (int i = 0; i < numEntries; i++) {
            hb_ot_name_entry_t entry = entries[i];
            QString lang(hb_language_to_string(entry.language));
            uint length = hb_ot_name_get_utf8(hbFace, entry.name_id, entry.language, nullptr, nullptr)+1;
            char buff[length];
            hb_ot_name_get_utf8(hbFace, entry.name_id, entry.language, &length, buff);
            QString name = QString::fromUtf8(buff, length);

            if (entry.name_id == HB_OT_NAME_ID_FONT_FAMILY) {
                ribbiFamilyNames.insert(lang, name);
            } else if (entry.name_id == HB_OT_NAME_ID_TYPOGRAPHIC_FAMILY) {
                typographicFamilyNames.insert(lang, name);
            } else if (entry.name_id == HB_OT_NAME_ID_WWS_FAMILY) {
                WWSFamilyNames.insert(lang, name);
            } else if (axisNameIDs.keys().contains(entry.name_id)) {
                fontFamily.axes[axisNameIDs.value(entry.name_id)].localizedLabels.insert(lang, name);
            } else if (entry.name_id == sizeNameId) {
                fontFamily.sizeInfo.localizedLabels.insert(lang, name);
            }
        }
        if (!typographicFamilyNames.isEmpty()) {
            typographicFamily.fontFamily = typographicFamilyNames.value("en", typographicFamilyNames.values().first());
            typographicFamily.localizedFontFamilies = typographicFamilyNames;
        }

        if (!ribbiFamilyNames.isEmpty()) {
            fontFamily.fontFamily = ribbiFamilyNames.value("en", ribbiFamilyNames.values().first());
            fontFamily.localizedFontFamilies = ribbiFamilyNames;
        }

        if (!WWSFamilyNames.isEmpty()) {
            wwsFamily.fontFamily = WWSFamilyNames.value("en", WWSFamilyNames.values().first());
            wwsFamily.localizedFontFamilies = WWSFamilyNames;
        }


    }
    if (typographicFamily.fontFamily.isEmpty()) {
        typographicFamily.fontFamily = fontFamily.fontFamily;
    }

    qDebug() << "adding..." << fontFamily.fontFamily << fontFamily.fileName;

    if (typographicFamily.fontFamily.isEmpty() && fontFamily.fontFamily.isEmpty()) {
        d->fontFamilyCollection.insert(d->fontFamilyCollection.childEnd(), fontFamily);
    } else {
        auto it = d->fontFamilyCollection.begin();
        for (; it != d->fontFamilyCollection.end(); it++) {
            if (!typographicFamily.fontFamily.isEmpty() && it->fontFamily == typographicFamily.fontFamily) {
                break;
            } else if (it->fontFamily == fontFamily.fontFamily) {
                break;
            }
        }
        if (it != d->fontFamilyCollection.end()) {
            d->fontFamilyCollection.insert(childEnd(it), fontFamily);
        } else {
            auto typographic = d->fontFamilyCollection.insert(d->fontFamilyCollection.childEnd(), typographicFamily);
            d->fontFamilyCollection.insert(childEnd(typographic), fontFamily);
        }
    }
    return true;
}

void KoFFWWSConverter::debugInfo()
{
    qDebug() << "Debug for font family collection" << KisForestDetail::size(d->fontFamilyCollection);
    QString spaces;
    for (auto it = compositionBegin(d->fontFamilyCollection); it != compositionEnd(d->fontFamilyCollection); it++) {
        if (it.state() == KisForestDetail::Enter) {
            qDebug().noquote() << QString(spaces + "+") << it->debugInfo();
            spaces.append("\t");
        }

        if (it.state() == KisForestDetail::Leave) {
            spaces.chop(1);
        }
    }
}


