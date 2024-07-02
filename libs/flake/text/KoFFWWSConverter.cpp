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

QStringList KoFontFamilyNode::debugInfo()
{
    QString style = isItalic? isOblique? "Oblique": "Italic": "Roman";
    QStringList debug = {QString("\'%1\' \'%2\', style: %3").arg(fontFamily, fontStyle, style)};
    debug.append(QString("Index: %1, File: %2").arg(fileIndex).arg(fileName));
    for (int i=0; i< axes.size(); i++) {
        KoFontFamilyAxis axis = axes.value(axes.keys().at(i));
        debug.append(axis.debugInfo());
    }
    for (int i=0; i< styleInfo.size(); i++) {
        debug.append(styleInfo.at(i).debugInfo());
    }
    if (sizeInfo.low >= 0 && sizeInfo.high >= 0) {
        debug.append(sizeInfo.debugInfo());
    }
    if (!pixelSizes.isEmpty()) {
        QStringList pix;
        for (int i=0; i< pixelSizes.size(); i++) {
            pix.append(QString::number(pixelSizes.keys().at(i)));
        }
        debug.append("PixelSizes: "+pix.join(", "));
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

constexpr unsigned OS2_ITALIC = 1u << 0;
constexpr unsigned OS2_BOLD = 1u << 5;
constexpr unsigned OS2_REGULAR = 1u << 6;
constexpr unsigned OS2_WWS = 1u << 8;
constexpr unsigned OS2_OBLIQUE = 1u << 9;
constexpr unsigned OS2_USE_TYPO_METRICS = 1u << 7;

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

    if (fontFamily.fileIndex > 0xffff) { // this indicates the font is a variable font instance, so we don't try to load it.
        return false;
    }
    for (auto it = d->fontFamilyCollection.compositionBegin(); it != d->fontFamilyCollection.compositionEnd(); it++) {
        if (it->fileName == fontFamily.fileName && it->fileIndex == fontFamily.fileIndex) {
            return true;
        }
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
            fontFamily.pixelSizes.insert((face->available_sizes[i].size / 64.0), {fontFamily.fileName});
        }
    } else {
        fontFamily.type = OpenType;
        hb_face_t *hbFace = hb_ft_face_create_referenced(face.data());

        // Retrieve width, weight and slant data.
        TT_OS2 *os2Table = nullptr;
        os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
        if (os2Table) {
            fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(os2Table->usWeightClass));
            fontFamily.axes.insert("wdth", KoFontFamilyAxis::widthAxis(percentageFromUsWidthClass(os2Table->usWidthClass)));
            if (os2Table->fsSelection & OS2_BOLD && os2Table->usWeightClass == 400) {
                fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(700));
            }
            fontFamily.isItalic = (os2Table->fsSelection & OS2_ITALIC);
            fontFamily.isOblique = (os2Table->fsSelection & OS2_OBLIQUE);
            if (os2Table->fsSelection & OS2_REGULAR) {
                fontFamily.axes.insert("wght", KoFontFamilyAxis::weightAxis(400));
                fontFamily.isItalic = false;
                fontFamily.isOblique = false;
            }
            if (os2Table->version >= 5) {
                FontFamilySizeInfo sizeInfo;
                qreal twip = 0.05; ///< twip is 'Twenty-in-point';
                sizeInfo.high = os2Table->usUpperOpticalPointSize * twip;
                sizeInfo.low = os2Table->usLowerOpticalPointSize * twip;
                sizeInfo.os2table = true;
                fontFamily.sizeInfo = sizeInfo;
            }
            isWWSFamilyWithoutName = (os2Table->fsSelection & OS2_WWS);
        }

        // retrieve gpos size data...
        uint designSize;
        uint subFamilyId;
        uint rangeStart;
        uint rangeEnd;
        hb_ot_name_id_t sizeNameId;
        if (hb_ot_layout_get_size_params(hbFace, &designSize, &subFamilyId, &sizeNameId, &rangeStart, &rangeEnd)) {
            FontFamilySizeInfo sizeInfo;
            qreal tenth = 0.1;
            sizeInfo.low = rangeStart * tenth;
            sizeInfo.high = rangeEnd * tenth;
            sizeInfo.subFamilyID = subFamilyId;
            sizeInfo.designSize = designSize * tenth;
            fontFamily.sizeInfo = sizeInfo;
        }

        // retrieve axis data...
        // Would also be good if we could read the STAT table for more info, but we cannot as there's no API for that in harfbuzz.

        QHash<hb_ot_name_id_t, QString> axisNameIDs;
        QVector<hb_ot_name_id_t> instanceNameIDs;
        if (hb_ot_var_has_data(hbFace)) {
            fontFamily.isVariable = true;
            uint count = hb_ot_var_get_axis_count(hbFace);
            uint maxInfos = 1;
            QStringList axesTags;
            for (uint i = 0; i < count; i++) {
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
                axisInfo.variableAxis = true;
                fontFamily.axes.insert(axisInfo.tag, axisInfo);
                axesTags.append(axisInfo.tag);
            }
            count = hb_ot_var_get_named_instance_count (hbFace);
            for (uint i = 0; i < count; i++) {
                QHash<QString, float> instanceCoords;
                uint coordLength = axesTags.size();
                float coordinate[coordLength];
                hb_ot_var_named_instance_get_design_coords (hbFace, i, &coordLength, coordinate);
                for (uint j =0; j < coordLength; j++ ){
                    instanceCoords.insert(axesTags.value(j), coordinate[j]);
                }
                FontFamilyStyleInfo style;
                style.instanceCoords = instanceCoords;
                instanceNameIDs.append(hb_ot_var_named_instance_get_subfamily_name_id(hbFace, i));
                fontFamily.styleInfo.append(style);
            }

        }

        // Get some basic color data.
        fontFamily.colorBitMap = hb_ot_color_has_png(hbFace);
        fontFamily.colorSVG = hb_ot_color_has_svg(hbFace);
        fontFamily.colorClrV0 = hb_ot_color_has_layers(hbFace);
        //fontFamily.colorClrV1 = hb_ot_color_has_paint(hbFace);

        uint numEntries = 0;
        const hb_ot_name_entry_t *entries = hb_ot_name_list_names(hbFace, &numEntries);

        QHash<QString, QString> ribbiFamilyNames;
        QHash<QString, QString> ribbiStyleNames;
        QHash<QString, QString> WWSFamilyNames;
        QHash<QString, QString> WWSStyleNames;
        QHash<QString, QString> typographicFamilyNames;
        QHash<QString, QString> typographicStyleNames;
        for (uint i = 0; i < numEntries; i++) {
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
            } else if (entry.name_id == HB_OT_NAME_ID_FONT_SUBFAMILY) {
                ribbiStyleNames.insert(lang, name);
            } else if (entry.name_id == HB_OT_NAME_ID_TYPOGRAPHIC_SUBFAMILY) {
                typographicStyleNames.insert(lang, name);
            } else if (entry.name_id == HB_OT_NAME_ID_WWS_SUBFAMILY) {
                WWSStyleNames.insert(lang, name);
            } else if (axisNameIDs.keys().contains(entry.name_id)) {
                fontFamily.axes[axisNameIDs.value(entry.name_id)].localizedLabels.insert(lang, name);
            } else if (entry.name_id == sizeNameId) {
                fontFamily.sizeInfo.localizedLabels.insert(lang, name);
            } else if (instanceNameIDs.contains(entry.name_id)) {
                int idx = instanceNameIDs.indexOf(entry.name_id);
                fontFamily.styleInfo[idx].localizedLabels.insert(lang, name);
            }
        }
        if (!typographicFamilyNames.isEmpty()) {
            typographicFamily.fontFamily = typographicFamilyNames.value("en", typographicFamilyNames.values().first());
            typographicFamily.localizedFontFamilies = typographicFamilyNames;
        }
        if (!typographicStyleNames.isEmpty()) {
            typographicFamily.fontStyle = typographicStyleNames.value("en", typographicStyleNames.values().first());
            typographicFamily.localizedFontStyle = typographicStyleNames;
        }
        if (!ribbiFamilyNames.isEmpty()) {
            fontFamily.fontFamily = ribbiFamilyNames.value("en", ribbiFamilyNames.values().first());
            fontFamily.localizedFontFamilies = ribbiFamilyNames;
        }
        if (!ribbiStyleNames.isEmpty()) {
            fontFamily.fontStyle = ribbiStyleNames.value("en", ribbiStyleNames.values().first());
            fontFamily.localizedFontStyle = ribbiStyleNames;
        }
        if (!WWSFamilyNames.isEmpty()) {
            wwsFamily.fontFamily = WWSFamilyNames.value("en", WWSFamilyNames.values().first());
            wwsFamily.localizedFontFamilies = WWSFamilyNames;
        }
        if (!WWSStyleNames.isEmpty()) {
            wwsFamily.fontStyle = WWSStyleNames.value("en", WWSStyleNames.values().first());
            wwsFamily.localizedFontFamilies = WWSStyleNames;
        }

        hb_face_destroy(hbFace);
    }
    if (typographicFamily.fontFamily.isEmpty()) {
        typographicFamily.fontFamily = fontFamily.fontFamily;
    }

    qDebug() << "adding..." << fontFamily.fontFamily << fontFamily.fileName;

    if (typographicFamily.fontFamily.isEmpty() && fontFamily.fontFamily.isEmpty()) {
        d->fontFamilyCollection.insert(d->fontFamilyCollection.childEnd(), fontFamily);
    } else {
        // find potential typographic family
        auto it = d->fontFamilyCollection.childBegin();
        for (; it != d->fontFamilyCollection.childEnd(); it++) {
            if (!typographicFamily.fontFamily.isEmpty() && it->fontFamily == typographicFamily.fontFamily) {
                break;
            } else if (it->fontFamily == fontFamily.fontFamily) {
                break;
            }
        }
        if (it != d->fontFamilyCollection.childEnd()) {

            if (isWWSFamilyWithoutName) {
                wwsFamily.fontFamily = fontFamily.fontFamily;
            }
            if (!wwsFamily.fontFamily.isEmpty()) {
                // sort into wws family
                auto wws = childBegin(it);
                for (; wws != childEnd(it); wws++) {
                    if (wws->fontFamily == wwsFamily.fontFamily) {
                        break;
                    }
                }
                if (wws != childEnd(it)) {
                    d->fontFamilyCollection.insert(childEnd(wws), fontFamily);
                } else {
                    auto wwsNew = d->fontFamilyCollection.insert(childEnd(it), wwsFamily);
                    d->fontFamilyCollection.insert(childEnd(wwsNew), fontFamily);
                }
            } else if (!fontFamily.pixelSizes.isEmpty()) {
                // sort any pixel sizes into the appropriate family.
                auto pixel = childBegin(it);
                for (; pixel != childEnd(it); pixel++) {
                    if (pixel->fontFamily == fontFamily.fontFamily && pixel->fontStyle == fontFamily.fontStyle && !pixel->pixelSizes.isEmpty()) {
                        for (int pxSize = 0; pxSize < fontFamily.pixelSizes.keys().size(); pxSize++) {
                            int px = fontFamily.pixelSizes.keys().at(pxSize);
                            QStringList files = pixel->pixelSizes.value(px, QStringList());
                            files.append(fontFamily.pixelSizes.value(px));
                            pixel->pixelSizes.insert(px, files);
                        }
                        break;
                    }
                }
                if (pixel == childEnd(it)) {
                    d->fontFamilyCollection.insert(childEnd(it), fontFamily);
                }

            } else {
                d->fontFamilyCollection.insert(childEnd(it), fontFamily);
            }
        } else {
            auto typographic = d->fontFamilyCollection.insert(d->fontFamilyCollection.childEnd(), typographicFamily);
            if (isWWSFamilyWithoutName) {
                wwsFamily.fontFamily = fontFamily.fontFamily;
            }
            if (!wwsFamily.fontFamily.isEmpty()) {
                auto wwsNew = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                d->fontFamilyCollection.insert(childEnd(wwsNew), fontFamily);
            } else {
                d->fontFamilyCollection.insert(childEnd(typographic), fontFamily);
            }
        }
    }
    return true;
}

void KoFFWWSConverter::sortIntoWWSFamilies()
{
    for (auto typographic = d->fontFamilyCollection.childBegin(); typographic != d->fontFamilyCollection.childEnd(); typographic++) {
        KisForest<KoFontFamilyNode> tempList;
        QVector<qreal> weights;
        QVector<qreal> widths;

        for (auto child = childBegin(typographic); child != childEnd(typographic); child++) {
            if (childBegin(child) != childEnd(child)) {
                continue;
            }
            tempList.insert(tempList.childEnd(), *child);
            qreal wght = child->axes.value("wght", KoFontFamilyAxis::weightAxis(400)).value;
            if (!weights.contains(wght)) weights.append(wght);
            qreal wdth = child->axes.value("wdth", KoFontFamilyAxis::widthAxis(100)).value;
            if (!widths.contains(wdth)) widths.append(wdth);

            d->fontFamilyCollection.erase(child);
        }
        //split up in
        if (KisForestDetail::size(tempList) > 0) {
            for (auto font = tempList.childBegin(); font != tempList.childEnd(); font++) {
                qreal testWeight = weights.contains(400)? 400: weights.first();
                qreal testWidth = widths.contains(100)? 100: widths.first();
                bool widthTested = !font->axes.keys().contains("wdth");
                widthTested = widthTested? true: font->axes.value("wdth").value == testWidth;

                if (font->axes.value("wght").value == testWeight && widthTested
                        && !font->isItalic && !font->isOblique) {
                    KoFontFamilyNode wwsFamily;
                    wwsFamily.fontFamily = font->fontFamily;
                    wwsFamily.fontStyle = font->fontStyle;
                    auto newWWS = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                    d->fontFamilyCollection.insert(childEnd(newWWS), *font);
                    tempList.erase(font);
                }
            }
            for (auto font = tempList.childBegin(); font != tempList.childEnd(); font++) {
                qDebug() << "testing" << font->fontFamily;
                auto wws = childBegin(typographic);
                for (; wws != childEnd(typographic); wws++) {
                    if (wws->fontFamily != font->fontFamily) {
                        continue;
                    }
                    if (font->type != OpenType) {
                        // Hack for really old fonts.
                        if (wws->fontStyle.toLower() != "regular"
                                && !font->fontStyle.contains(wws->fontStyle)) {
                            continue;
                        }
                    }
                    auto wwsChild = childBegin(wws);
                    for (; wwsChild != childEnd(wws); wwsChild++) {

                        if (wwsChild->isItalic == font->isItalic
                                && wwsChild->isOblique == font->isOblique
                                && wwsChild->compareAxes(font->axes)) {
                            break;
                        }
                    }
                    if (wwsChild != childEnd(wws)) {
                        continue;
                    } else {
                        d->fontFamilyCollection.insert(childEnd(wws), *font);
                        break;
                    }
                }
                if (wws == childEnd(typographic)) {
                    KoFontFamilyNode wwsFamily;
                    wwsFamily.fontFamily = font->fontFamily;
                    wwsFamily.fontStyle = font->fontStyle;
                    auto newWWS = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                    d->fontFamilyCollection.insert(childEnd(newWWS), *font);
                }
            }
        }

    }
}

void KoFFWWSConverter::debugInfo()
{
    qDebug() << "Debug for font family collection" << KisForestDetail::size(d->fontFamilyCollection);
    QString spaces;
    for (auto it = compositionBegin(d->fontFamilyCollection); it != compositionEnd(d->fontFamilyCollection); it++) {
        if (it.state() == KisForestDetail::Enter) {
            QStringList debugInfo = it->debugInfo();
            for (int i = 0; i< debugInfo.size(); i++) {
                if (i==0) {
                    qDebug().noquote() << QString(spaces + "+") << debugInfo.at(i);
                } else {
                    qDebug().noquote() << QString(spaces + "| ") << debugInfo.at(i);
                }
            }
            spaces.append("    ");
        }

        if (it.state() == KisForestDetail::Leave) {
            spaces.chop(4);
        }
    }
}


