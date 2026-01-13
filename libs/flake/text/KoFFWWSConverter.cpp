/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFFWWSConverter.h"

#include <KisForest.h>
#include <KisStaticInitializer.h>
#include <kis_assert.h>
#include <hb.h>
#include <hb-ft.h>
#include FT_TRUETYPE_TABLES_H

#include <QFileInfo>


/**
 * @brief The FontFamilySizeInfo class
 * Some font-families have different designs for different sizes. These are largely
 * differences in weight, spacing and small glyph changes.
 * There's four places opentype stores the design size information:
 * 1. Different bitmap strikes in the font file. Bitmap fonts do the same.
 * 2. The OS2 table entry.
 * 3. The 'size' opentype feature.
 * 4. The 'opsz' axes in either variable fonts or the stat table.
 *
 * Of these, 1 and 4 are supported properly, but we need to keep track of 2 and 3 as well,
 * to ensure that these fonts are still selectable in the font picker.
 */
struct FontFamilySizeInfo {
    bool isSet = false; /// Whether the size info is set.
    bool os2table = false; /// Whether this is using the OS2 table or the GPOS Size feature.
    int subFamilyID = 0;

    qreal low = -1;
    qreal high = -1;
    qreal designSize = 0;
    QHash<QLocale, QString> localizedLabels;
    QString debugInfo() const {
        QString label;
        if (!localizedLabels.isEmpty()) {
            label = localizedLabels.value(QLocale(QLocale::English), localizedLabels.values().first());
        }
        return QString("Optical Size Info: OS2=%1, label: %2, min: %3, max: %4, designSize: %5").arg(os2table? "true": "false").arg(label).arg(low).arg(high).arg(designSize);
    }

    bool compare(const FontFamilySizeInfo &other) {
        if (isSet != other.isSet) return false;
        if (os2table != other.os2table) {
            return false;
        } else if (os2table) {
            return qFuzzyCompare(low, other.low) && qFuzzyCompare(high, other.high);
        } else {
            return qFuzzyCompare(designSize, other.designSize);
        }
    }
};

QDebug operator<<(QDebug dbg, const FontFamilySizeInfo &info) {
    dbg.nospace() << info.debugInfo();
    return dbg.space();
}

struct FontFamilyNode {

    FontFamilyNode() {}

    QString fontFamily;
    QString fontStyle;
    QString fileName;
    int fileIndex = 0; /// Truetype collections have indices that need to be checked against.

    QHash<QString, QString> sampleStrings; /// sample string used to generate the preview;
    QList<QLocale> supportedLanguages; /// Languages supported, according to fontconfig.

    QStringList otherFiles; /// Other files that seem related. These might be duplicate font files, or fonts where only the tech differs.
    QDateTime lastModified; /// Last time the file was modified.

    // The localized font-families. This should be the name associated with the current node,
    // and thus is the typographic, wws or ribbi name depending on the depth.
    QHash<QLocale, QString> localizedFontFamilies;

    // Style name can depend on depth, and when returning the representation, we need to select the correct name.
    QHash<QLocale, QString> localizedFontStyle;
    QHash<QLocale, QString> localizedTypographicStyle;
    QHash<QLocale, QString> localizedWWSStyle;

    // The full proper name as used by Windows to identify unique names.
    QHash<QLocale, QString> localizedFullName;

    /**
     * @brief axes
     * While typical font-files within the same family are defined by having a single weight or width,
     * variable fonts are defined by a range. The axis info abstracts both into a structure of a range
     * between two values, which in turn means that if we take a family as a whole, we can combine
     * all axes to find the total range of variations.
     */
    QHash<QString, KoSvgText::FontFamilyAxis> axes;

    /**
     * @brief styleInfo
     * This abstracts both font families that consist of many separate font-files and variable fonts
     * with many separate instances, and the hybrid of the two (commonly, if there's an italic type,
     * it is put into a separate variable font file from the regular type).
     */
    QList<KoSvgText::FontFamilyStyleInfo> styleInfo;

    /**
     * @brief pixelSizes
     * This is only used for bitmap fonts, when searching we try to return the files associated with the
     * appropriate pixelsize first, so that usage wise it'll feel identical to using an opentype font
     * with multiple bitmap strikes.
     *
     * int: the pixel size.
     * QStringList: list of files associated with that pixel-size.
     */
    QHash<int, QStringList> pixelSizes;

    /**
     * @brief sizeInfo
     * This is only really used to ensure that sizes get sorted into different WWS families, as otherwise they're unselectable.
     * We could take them into account when searching, but the CSS-WG hasn't explicitly requested or provided any guidance therein.
     */
    FontFamilySizeInfo sizeInfo;

    // Data from the osTable fsSelection flags. Older fonts use these to identify italic/oblique in ways that are hard
    // to coalesce with the axes information, though it'd be good if we could figure out how.
    bool isItalic = false;
    bool isOblique = false;

    bool compareAxes(QHash<QString, KoSvgText::FontFamilyAxis> otherAxes) {
        if (axes.keys() != otherAxes.keys()) {
            return false;
        }
        for (int k = 0; k < axes.keys().size(); k++) {
            KoSvgText::FontFamilyAxis a = axes.value(axes.keys().at(k));
            KoSvgText::FontFamilyAxis b = otherAxes.value(axes.keys().at(k));
            if (!qFuzzyCompare(a.value, b.value)) {
                return false;
            }
        }
        return true;
    }

    static FontFamilyNode createWWSFamilyNode(const FontFamilyNode &child, const FontFamilyNode &typographic, QStringList existingWWSNames) {
        FontFamilyNode wwsFamily;
        if (child.type != KoSvgText::OpenTypeFontType) {
            if (child.fontStyle.toLower() == "regular") {
                wwsFamily.fontFamily = child.fontFamily;
            } else {
                wwsFamily.fontFamily = child.fontFamily + " " + child.fontStyle;
            }
            wwsFamily.fontStyle = child.fontStyle;
        } else {
            wwsFamily.fontFamily = typographic.fontFamily;
            if (existingWWSNames.contains(typographic.fontFamily)) {
                wwsFamily.fontFamily = child.fontFamily;
                if (existingWWSNames.contains(child.fontFamily)) {
                    wwsFamily.fontFamily = child.fontFamily + " " + child.fontStyle;
                }
            }
        }
        wwsFamily.localizedTypographicStyle = child.localizedTypographicStyle;
        wwsFamily.localizedFontFamilies = child.localizedFontFamilies;
        wwsFamily.isVariable = child.isVariable;
        wwsFamily.colorBitMap = child.colorBitMap;
        wwsFamily.colorSVG = child.colorSVG;
        wwsFamily.colorClrV0 = child.colorClrV0;
        wwsFamily.colorClrV1 = child.colorClrV1;
        wwsFamily.type = child.type;
        return wwsFamily;
    }

    KoSvgText::FontFormatType type = KoSvgText::UnknownFontType;
    bool isVariable = false;
    bool colorClrV0 = false;
    bool colorClrV1 = false;
    bool colorSVG = false;
    bool colorBitMap = false;

    bool hasAnyColor() const {
        return (colorClrV0 || colorClrV1 || colorSVG || colorBitMap);
    }

    QStringList debugInfo() const;
};

QStringList FontFamilyNode::debugInfo() const
{
    const QString style = isItalic? isOblique? "Oblique": "Italic": "Roman";
    const QString fullname = localizedFullName.empty()? "": localizedFullName.values().first();
    QStringList debug = {QString("\'%1\' \'%2\', style: %3, type:%4, full name: %5").arg(fontFamily, fontStyle, style).arg(type).arg(fullname)};
    debug.append(QString("Index: %1, File: %2").arg(fileIndex).arg(fileName));
    for (int i=0; i< axes.size(); i++) {
        KoSvgText::FontFamilyAxis axis = axes.value(axes.keys().at(i));
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
    } else if (!otherFiles.isEmpty()) {
        debug.append("Other files: "+otherFiles.join(", "));
    }
    return debug;
}

QDebug operator<<(QDebug dbg, const FontFamilyNode &node) {
    dbg.nospace() << node.debugInfo();
    return dbg.space();
}

struct KoFFWWSConverter::Private {
    Private() {}

    /**
     * @brief fontFamilyCollection
     *
     * The main reason this WWS converter class exists, is because there's 3 major ways that font-families get sorted:
     * 1. RIBBI style, this means a single family can have a regular, a bold, an italic and a bold-italic.
     * 2. WWS style, which means that a single family can have variations in width, weight(bold), and slant (italic).
     * 3. Typographic style, which means a single family can have all sorts of variations,
     * limited only by the designer's imagination.
     *
     * In practice, this means that a single font file can have unique names for each of these three families, as different
     * systems may only support a certain type. Because CSS only really has controls for WWS and RIBBI style, we need to
     * untangle the font-families so we can identify the correct name for a given font within these restrictions.
     *
     * For this purpose, we create a font-family collection tree that is sorted as such:
     *
     * 1. Typographic
     *  2. WWS/RIBBI family
     *   3. Font file and identical alternates, the latter of which count when:
     *      - The file has the same family and sub family name (and same css values),
     *        which can happen when there's both type1 and opentype versions of a font.
     *        We prioritize opentype files here, but returning all these filenames means that the type1 files
     *        can be used during glyph-fallback.
     *      - The file is a bitmap font with different sizes. This in particular is so that usage
     *        will behave the exact same as an opentype file with multiple bitmap strikes.
     *
     * This tree then allows us to search on all 3 entries, in particular their family names (and their localized variants),
     * and will be able to fall-back on the other names if the name that the user selected is not the WWS/RIBBI name.
     * (For example, an artist sets the name of the font family to "Amstelvar", which is the typographic name, but the WWS
     * name is "Amstelvar Roman". This class will prioritize WWS values when searching, but still select the typographic name
     * if it cannot match the wws name. Similarly, if a full font name (yes, that exists too) has been used, this will be prioritized).
     */
    KisForest<FontFamilyNode> fontFamilyCollection;
};

KoFFWWSConverter::KoFFWWSConverter()
    : d(new Private())
{

}

KoFFWWSConverter::~KoFFWWSConverter()
{
}

// OS2 fsSelection bitflags. See https://learn.microsoft.com/en-us/typography/opentype/spec/os2#fsselection
constexpr unsigned OS2_ITALIC = 1u << 0; /// Is italic
constexpr unsigned OS2_BOLD = 1u << 5; /// Is bold.
constexpr unsigned OS2_REGULAR = 1u << 6; /// Is truly regular (instead of italic or oblique)
constexpr unsigned OS2_WWS = 1u << 8; /// Indicates that the given font is primarily a WWS family and requires no further processing.
constexpr unsigned OS2_OBLIQUE = 1u << 9; // Is an oblique instead of an italic.
constexpr unsigned OS2_USE_TYPO_METRICS = 1u << 7;

const QString WEIGHT_TAG = "wght";
const QString WIDTH_TAG = "wdth";
const QString SLANT_TAG = "slnt";
const QString ITALIC_TAG = "ital";
const QString OPTICAL_TAG = "opsz";

bool KoFFWWSConverter::addFontFromPattern(const FcPattern *pattern, FT_LibrarySP freeTypeLibrary)
{
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
    QString filename = QString::fromUtf8(reinterpret_cast<char *>(fileValue));

    int indexValue{};
    if (FcPatternGetInteger(pattern, FC_INDEX, 0, &indexValue) != FcResultMatch) {
        qWarning() << "Failed to get font index for" << pattern << "(file:" << filename << ")";
        getFile = false;
    }

    if (getFile == false) {
        return getFile;
    }

    if (indexValue > 0xffff) { // this indicates the font is a variable font instance, so we don't try to load it.
        return false;
    }

    bool success = addFontFromFile(filename, indexValue, freeTypeLibrary);
    if (success) {
        FcLangSet *set;
        if (FcPatternGetLangSet(pattern, FC_LANG, 0, &set) != FcResultMatch) {
            qWarning() << "Failed to get font index for" << pattern << "(file:" << filename << ")";
            return success;
        }
        FcStrList *list = FcStrListCreate(FcLangSetGetLangs(set));
        FcStrListFirst(list);
        FcChar8 *langString = FcStrListNext(list);
        QString lang = QString::fromUtf8(reinterpret_cast<char *>(langString));
        QList<QLocale> languages;
        while (!lang.isEmpty()) {
            languages.append(QLocale(lang));

            langString = FcStrListNext(list);
            lang = QString::fromUtf8(reinterpret_cast<char *>(langString));
        }
        FcStrListDone(list);
        FcCharSet *charSet = nullptr;
        if (FcPatternGetCharSet(pattern, FC_CHARSET, 0, &charSet) != FcResultMatch) {
            return success;
        }
        addSupportedLanguagesByFile(filename, indexValue, languages, charSet);

    }
    return success;
}

bool KoFFWWSConverter::addFontFromFile(const QString &filename, const int index, FT_LibrarySP freeTypeLibrary) {

    FontFamilyNode fontFamily;
    fontFamily.fileName = filename;
    fontFamily.fileIndex = index;
    for (auto it = d->fontFamilyCollection.begin(); it != d->fontFamilyCollection.end(); it++) {
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
    fontFamily.lastModified = QFileInfo(fontFamily.fileName).lastModified();
    FontFamilyNode typographicFamily;
    FontFamilyNode wwsFamily;
    bool isWWSFamilyWithoutName = false;

    if (!FT_IS_SFNT(face.data())) {
        fontFamily.type = FT_IS_SCALABLE(face.data())? KoSvgText::Type1FontType: KoSvgText::BDFFontType;

        fontFamily.isItalic = face->style_flags & FT_STYLE_FLAG_ITALIC;
        if (face->style_flags & FT_STYLE_FLAG_BOLD) {
            fontFamily.axes.insert(WEIGHT_TAG, KoSvgText::FontFamilyAxis::weightAxis(700));
        } else {
            fontFamily.axes.insert(WEIGHT_TAG, KoSvgText::FontFamilyAxis::weightAxis(400));
        }

        for (int i=0; i< face->num_fixed_sizes; i++) {
            // 64 = Freetype pixel
            fontFamily.pixelSizes.insert((face->available_sizes[i].size / 64.0), {fontFamily.fileName});
        }
    } else {
        fontFamily.type = KoSvgText::OpenTypeFontType;
        hb_face_t_sp hbFace(hb_ft_face_create_referenced(face.data()));
        hb_font_t_sp hbFont(hb_ft_font_create_referenced(face.data()));

        // Retrieve width, weight and slant data.

        fontFamily.axes.insert(WEIGHT_TAG, KoSvgText::FontFamilyAxis::weightAxis(hb_style_get_value(hbFont.data(), HB_STYLE_TAG_WEIGHT)));
        fontFamily.axes.insert(WIDTH_TAG, KoSvgText::FontFamilyAxis::widthAxis(hb_style_get_value(hbFont.data(), HB_STYLE_TAG_WIDTH)));
        fontFamily.isItalic = hb_style_get_value(hbFont.data(), HB_STYLE_TAG_ITALIC) > 0;
        //fontFamily.isOblique = hb_style_get_value(hbFont.data(), HB_STYLE_TAG_SLANT_ANGLE) != 0;

        TT_OS2 *os2Table = nullptr;
        os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
        if (os2Table) {

            fontFamily.isOblique = os2Table->fsSelection & OS2_OBLIQUE;

            if (os2Table->fsSelection & OS2_REGULAR) {
                fontFamily.isItalic = false;
                fontFamily.isOblique = false;
            }

            if (os2Table->version >= 5) {
                FontFamilySizeInfo sizeInfo;
                const qreal twip = 0.05; ///< twip is 'Twenty-in-point';
                sizeInfo.high = os2Table->usUpperOpticalPointSize * twip;
                sizeInfo.low = os2Table->usLowerOpticalPointSize * twip;
                sizeInfo.os2table = true;
                sizeInfo.isSet = true;
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
        if (hb_ot_layout_get_size_params(hbFace.data(), &designSize, &subFamilyId, &sizeNameId, &rangeStart, &rangeEnd)) {
            FontFamilySizeInfo sizeInfo;
            qreal tenth = 0.1;
            sizeInfo.low = rangeStart * tenth;
            sizeInfo.high = rangeEnd * tenth;
            sizeInfo.subFamilyID = subFamilyId;
            sizeInfo.designSize = designSize * tenth;
            sizeInfo.isSet = true;
            fontFamily.sizeInfo = sizeInfo;
        }

        // retrieve axis data...
        // Would also be good if we could read the STAT table for more info, but we cannot as there's no API for that in harfbuzz.

        QHash<hb_ot_name_id_t, QString> axisNameIDs;
        QVector<hb_ot_name_id_t> instanceNameIDs;
        if (hb_ot_var_has_data(hbFace.data())) {
            fontFamily.isVariable = true;
            uint count = hb_ot_var_get_axis_count(hbFace.data());
            uint maxInfos = 1;
            QStringList axesTags;
            for (uint i = 0; i < count; i++) {
                KoSvgText::FontFamilyAxis axisInfo;
                hb_ot_var_axis_info_t axis;
                hb_ot_var_get_axis_infos(hbFace.data(), i, &maxInfos, &axis);
                axisInfo.min = axis.min_value;
                axisInfo.max = axis.max_value;
                axisInfo.value = axis.default_value;
                axisInfo.axisHidden = axis.flags & HB_OT_VAR_AXIS_FLAG_HIDDEN;
                char buff[4];
                hb_tag_to_string(axis.tag, buff);
                axisInfo.tag = QString::fromLatin1(buff, 4);
                axisNameIDs.insert(axis.name_id, axisInfo.tag);
                axisInfo.variableAxis = true;
                fontFamily.axes.insert(axisInfo.tag, axisInfo);
                axesTags.append(axisInfo.tag);
            }
            count = hb_ot_var_get_named_instance_count (hbFace.data());
            for (uint i = 0; i < count; i++) {
                QHash<QString, float> instanceCoords;
                uint coordLength = axesTags.size();
                std::vector<float> coordinate(coordLength);
                hb_ot_var_named_instance_get_design_coords (hbFace.data(), i, &coordLength, coordinate.data());
                for (uint j =0; j < coordLength; j++ ){
                    instanceCoords.insert(axesTags.value(j), coordinate[j]);
                }
                KoSvgText::FontFamilyStyleInfo style;
                style.instanceCoords = instanceCoords;
                instanceNameIDs.append(hb_ot_var_named_instance_get_subfamily_name_id(hbFace.data(), i));
                fontFamily.styleInfo.append(style);
            }
        }

        // Get some basic color data.
        fontFamily.colorBitMap = hb_ot_color_has_png(hbFace.data());
        fontFamily.colorSVG = hb_ot_color_has_svg(hbFace.data());
        fontFamily.colorClrV0 = hb_ot_color_has_layers(hbFace.data());
        //fontFamily.colorClrV1 = hb_ot_color_has_paint(hbFace);
        wwsFamily.colorBitMap = fontFamily.colorBitMap;
        wwsFamily.colorSVG = fontFamily.colorSVG;
        wwsFamily.colorClrV0 = fontFamily.colorClrV0;

        uint numEntries = 0;
        const hb_ot_name_entry_t *entries = hb_ot_name_list_names(hbFace.data(), &numEntries);

        QHash<QLocale, QString> ribbiFamilyNames;
        QHash<QLocale, QString> ribbiStyleNames;
        QHash<QLocale, QString> WWSFamilyNames;
        QHash<QLocale, QString> WWSStyleNames;
        QHash<QLocale, QString> typographicFamilyNames;
        QHash<QLocale, QString> typographicStyleNames;
        QHash<QLocale, QString> fullNames;
        for (uint i = 0; i < numEntries; i++) {
            hb_ot_name_entry_t entry = entries[i];
            QString lang(hb_language_to_string(entry.language));
            QLocale locale(lang);
            uint length = hb_ot_name_get_utf8(hbFace.data(), entry.name_id, entry.language, nullptr, nullptr)+1;
            std::vector<char> buff(length);
            hb_ot_name_get_utf8(hbFace.data(), entry.name_id, entry.language, &length, buff.data());
            QString name = QString::fromUtf8(buff.data(), length);
            if (name.isEmpty()) continue;

            if (entry.name_id == HB_OT_NAME_ID_FONT_FAMILY) {
                ribbiFamilyNames.insert(locale, name);
            } else if (entry.name_id == HB_OT_NAME_ID_TYPOGRAPHIC_FAMILY) {
                typographicFamilyNames.insert(locale, name);
            } else if (entry.name_id == HB_OT_NAME_ID_WWS_FAMILY) {
                WWSFamilyNames.insert(locale, name);
            } else if (entry.name_id == HB_OT_NAME_ID_FONT_SUBFAMILY) {
                ribbiStyleNames.insert(locale, name);
            } else if (entry.name_id == HB_OT_NAME_ID_TYPOGRAPHIC_SUBFAMILY) {
                typographicStyleNames.insert(locale, name);
            } else if (entry.name_id == HB_OT_NAME_ID_WWS_SUBFAMILY) {
                WWSStyleNames.insert(locale, name);
            } else if (entry.name_id == HB_OT_NAME_ID_FULL_NAME) {
                fullNames.insert(locale, name);
            } else if (entry.name_id > 0) { // Fonts made by Adobe seem to use the copyright id (0) as the input when the given value is empty.
                if (axisNameIDs.keys().contains(entry.name_id)) {
                    fontFamily.axes[axisNameIDs.value(entry.name_id)].localizedLabels.insert(locale, name);
                } else if (entry.name_id == sizeNameId) {
                    fontFamily.sizeInfo.localizedLabels.insert(locale, name);
                } else if (instanceNameIDs.contains(entry.name_id)) {
                    int idx = instanceNameIDs.indexOf(entry.name_id);
                    fontFamily.styleInfo[idx].localizedLabels.insert(locale, name);
                }
            }
        }
        QLocale english(QLocale::English);
        if (!typographicFamilyNames.isEmpty()) {
            typographicFamily.fontFamily = typographicFamilyNames.value(english, typographicFamilyNames.values().first());
            typographicFamily.localizedFontFamilies = typographicFamilyNames;
        }
        fontFamily.localizedTypographicStyle = typographicStyleNames;
        if (!ribbiFamilyNames.isEmpty()) {
            fontFamily.fontFamily = ribbiFamilyNames.value(english, ribbiFamilyNames.values().first());
            fontFamily.localizedFontFamilies = ribbiFamilyNames;
        }
        // Second check is a hack to avoid issues with the css test fonts. Why they are configure this way, beats me.
        // Either way, if the fullname, which has 100% priority is the name as typographic name (which is tested last)
        // the font search will only select the node with this fullname, which we don't want.
        if (!fullNames.isEmpty() && fullNames != typographicFamilyNames) {
            fontFamily.localizedFullName = fullNames;
        }
        if (!ribbiStyleNames.isEmpty()) {
            fontFamily.fontStyle = ribbiStyleNames.value(english, ribbiStyleNames.values().first());
            fontFamily.localizedFontStyle = ribbiStyleNames;
        }
        if (!WWSFamilyNames.isEmpty()) {
            wwsFamily.fontFamily = WWSFamilyNames.value(english, WWSFamilyNames.values().first());
            wwsFamily.localizedFontFamilies = WWSFamilyNames;
        }
        fontFamily.localizedWWSStyle = WWSStyleNames;
    }

    if (fontFamily.fontFamily.isEmpty()) {
        fontFamily.fontFamily = QFileInfo(fontFamily.fileName).baseName();
    }
    if (typographicFamily.fontFamily.isEmpty()) {
        typographicFamily.fontFamily = fontFamily.fontFamily;
    }
    wwsFamily.isVariable = fontFamily.isVariable;
    wwsFamily.type = fontFamily.type;
    typographicFamily.type = typographicFamily.type;

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

#include <KoWritingSystemUtils.h>
void KoFFWWSConverter::addSupportedLanguagesByFile(const QString &filename, const int index, const QList<QLocale> &supportedLanguages, FcCharSet *set)
{
    auto it = d->fontFamilyCollection.depthFirstTailBegin();
    for (; it!= d->fontFamilyCollection.depthFirstTailEnd(); it++) {
        if (it->fileName == filename && it->fileIndex == index) {
            break;
        }
    }
    if (it != d->fontFamilyCollection.depthFirstTailEnd()) {
        it->supportedLanguages = supportedLanguages;

        QMap<QString, QString> samples = KoWritingSystemUtils::samples();

        for (int i = 0; i < samples.size(); i++) {
            QString sample = samples.keys().at(i);
            bool matching = true;
            Q_FOREACH (uint unicode, sample.toUcs4()) {
                if (!FcCharSetHasChar(set, unicode)) {
                    matching = false;
                    break;
                }
            }
            if (matching) {
                it->sampleStrings.insert(samples.value(sample), sample);
            }
        }
    }
}

void KoFFWWSConverter::sortIntoWWSFamilies()
{
    QStringList wwsNames;
    // Some font families have predefined wws families, others don't. This function sorts out everything so that each font file has
    // a wws family in between the typographic and font-file nodes, this is important, because the wws family will be the one presented
    // as the font-family resource.
    for (auto typographic = d->fontFamilyCollection.childBegin(); typographic != d->fontFamilyCollection.childEnd(); typographic++) {
        KisForest<FontFamilyNode> tempList;

        QVector<qreal> weights;
        QVector<qreal> widths;
        QVector<int> fileIndices;
        QVector<KoSvgText::FontFormatType> types;

        // This takes all of the current children that aren't inside a wws-node already and puts them into a temp list,
        // as well as tallying the current widths and weights. We need these to find the most regular value.
        QVector<KisForest<FontFamilyNode>::child_iterator> deleteList;
        for (auto child = childBegin(typographic); child != childEnd(typographic); child++) {
            if (childBegin(child) != childEnd(child)) {
                wwsNames.append(child->fontFamily);
                continue;
            }
            tempList.insert(tempList.childEnd(), *child);
            qreal wght = child->axes.value(WEIGHT_TAG, KoSvgText::FontFamilyAxis::weightAxis(400)).value;
            if (!weights.contains(wght)) weights.append(wght);
            qreal wdth = child->axes.value(WIDTH_TAG, KoSvgText::FontFamilyAxis::widthAxis(100)).value;
            if (!widths.contains(wdth)) widths.append(wdth);
            if (!fileIndices.contains(child->fileIndex)) fileIndices.append(child->fileIndex);
            types.append(child->type);
            deleteList.append(child);
        }
        while (!deleteList.isEmpty()) {
            auto child = deleteList.takeFirst();
            KIS_ASSERT_RECOVER_NOOP(childBegin(child) == childEnd(child));
            d->fontFamilyCollection.erase(child);
        }
        if (KisForestDetail::size(tempList) > 0) {
            //Do most regular first...
            KoSvgText::FontFormatType testType = types.contains(KoSvgText::OpenTypeFontType)? KoSvgText::OpenTypeFontType: types.first();
            QVector<QPair<QString, QString>> existing;
            for (auto font = tempList.childBegin(); font != tempList.childEnd(); font++) {
                const qreal testWeight = weights.contains(400)? 400: weights.first();
                const qreal testWidth = widths.contains(100)? 100: widths.first();
                // We want to select the first file index if possible.
                const int testFileIndex = fileIndices.contains(0)? 0: fileIndices.first();

                bool widthTested = !font->axes.keys().contains(WIDTH_TAG);
                widthTested = widthTested? true: qFuzzyCompare(font->axes.value(WIDTH_TAG).value, testWidth);

                QPair<QString, QString> fontStyle(font->fontFamily, font->fontStyle);
                if (qFuzzyCompare(font->axes.value(WEIGHT_TAG).value, testWeight) && widthTested
                        && !font->isItalic
                        && !font->isOblique
                        && ( ( fileIndices.size() > 1 && font->fileIndex == testFileIndex ) || ( fileIndices.size()<=1 ))
                        && font->type == testType
                        && !existing.contains(fontStyle)) {
                    FontFamilyNode wwsFamily = FontFamilyNode::createWWSFamilyNode(*font, *typographic, wwsNames);
                    wwsNames.append(wwsFamily.fontFamily);
                    existing.append(fontStyle);

                    auto newWWS = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                    d->fontFamilyCollection.insert(childEnd(newWWS), *font);
                    deleteList.append(font);
                }
            }
            while (!deleteList.isEmpty()) {
                auto child = deleteList.takeFirst();
                KIS_ASSERT_RECOVER_NOOP(childBegin(child) == childEnd(child));
                tempList.erase(child);
            }
            // Then sort the rest of the family nodes into wws families.
            for (auto font = tempList.childBegin(); font != tempList.childEnd(); font++) {
                auto wws = childBegin(typographic);
                auto wwsCandidate = childEnd(typographic);
                for (; wws != childEnd(typographic); wws++) {
                    auto wwsChild = childBegin(wws);
                    if (font->type != KoSvgText::OpenTypeFontType && font->type != KoSvgText::Type1FontType) {
                        // Hack for really old fonts.
                        // It's questionable whether this is wise, given that it is not the family name,
                        // but it seems CSS is explicitly vague about what the family name is, because
                        // of the varying ways a font can be assigned a family name.
                        // Like, it is technically correct, because it allows us to select fonts with CSS
                        // that would otherwise be unselectable, but this does mean that other applications
                        // would need to have our exact idea of a CSS family name.
                        if (wws->fontStyle.toLower() != "regular"
                                && !font->fontStyle.contains(wws->fontStyle)) {
                            continue;
                        }
                    }
                    // In a previous version of the code, variable and non-variable were not mixed, but after reconsidering,
                    // they probably should be sorted together. The code will prioritize variable fonts in any case.

                    if (!wwsChild->sizeInfo.compare(font->sizeInfo)) {
                        // Skip sorting if the WWS family has size info that is incompatible with the sorted font.
                        continue;
                    }
                    for (; wwsChild != childEnd(wws); wwsChild++) {
                        if (wwsChild->isItalic == font->isItalic
                                && wwsChild->isOblique == font->isOblique
                                && wwsChild->compareAxes(font->axes)
                                && wwsChild->hasAnyColor() == font->hasAnyColor()) {
                            break;
                        }
                    }
                    if (wwsChild != childEnd(wws)) {
                        if (wwsChild->fontFamily == font->fontFamily && wwsChild->fontStyle == font->fontStyle) {
                            // If, for all intends and purposes, the font seems to be the same, merge nodes.
                            // This sometimes happens with installations where the same font is installed in
                            // a variety of formats. We want to prefer the opentype version in any case.
                            if (wwsChild->type != KoSvgText::OpenTypeFontType && font->type == KoSvgText::OpenTypeFontType) {
                                wwsChild->otherFiles.append(wwsChild->fileName);
                                wwsChild->otherFiles.append(font->otherFiles);
                                wwsChild->fileName = font->fileName;
                                wwsChild->type = KoSvgText::OpenTypeFontType;
                            } else {
                                wwsChild->otherFiles.append(font->fileName);
                                wwsChild->otherFiles.append(font->otherFiles);
                            }
                            if (wwsChild->lastModified < font->lastModified) {
                                wwsChild->lastModified = font->lastModified;
                            }
                            break;
                        } else {
                            continue;
                        }
                    } else {
                        /*
                         * Try to match the wws family with the same fontfamily, otherwise select the first candidate.
                         */
                        if (wwsCandidate == childEnd(typographic)) {
                            wwsCandidate = wws;
                        }
                        if (wws->fontFamily == font->fontFamily) {
                            wwsCandidate = wws;
                            break;
                        }
                    }
                }
                if (wwsCandidate != childEnd(typographic)) {
                    d->fontFamilyCollection.insert(childEnd(wwsCandidate), *font);
                } else if (wws == childEnd(typographic)) {
                    FontFamilyNode wwsFamily = FontFamilyNode::createWWSFamilyNode(*font, *typographic, wwsNames);
                    wwsNames.append(wwsFamily.fontFamily);
                    auto newWWS = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                    if (wwsFamily.fontFamily != typographic->fontFamily) {
                        font->localizedTypographicStyle.clear();
                    }
                    d->fontFamilyCollection.insert(childEnd(newWWS), *font);
                }
            }
            // This only triggers when the first wws family was created with the typographic name,
            // yet more wws families have followed after sorting was finished, and this name might be not the most precise.
            if (wwsNames.contains(typographic->fontFamily)  && std::distance(childBegin(typographic), childEnd(typographic)) > 1) {
                for (auto wws = childBegin(typographic); wws != childEnd(typographic); wws++) {
                    if (wws->fontFamily == typographic->fontFamily) {
                        if (wwsNames.contains(childBegin(wws)->fontFamily)) {
                            const QString otherWWSName = childBegin(wws)->fontFamily;
                            // Also rename any other wwsfamilies that has been using the second style.
                            for (auto otherwws = childBegin(typographic); otherwws != childEnd(typographic); otherwws++) {
                                if (otherwws->fontFamily == otherWWSName) {
                                    otherwws->fontFamily = childBegin(otherwws)->fontFamily + " " + childBegin(otherwws)->fontStyle;
                                    QHash<QLocale, QString> families;
                                    const QHash<QLocale, QString> styles = childBegin(otherwws)->localizedFontStyle;
                                    Q_FOREACH (const QLocale l, otherwws->localizedFontFamilies.keys()) {
                                        families.insert(l, otherwws->localizedFontFamilies.value(l)+" "+styles.value(l, childBegin(otherwws)->fontStyle));
                                    }
                                    otherwws->localizedFontFamilies = families;
                                    break;
                                }
                            }
                        }
                        wws->fontFamily = childBegin(wws)->fontFamily;
                        childBegin(wws)->localizedTypographicStyle.clear();
                        break;
                    }
                }
            }
        }
    }
}

void KoFFWWSConverter::addGenericFamily(const QString &name)
{
    FontFamilyNode typographicFamily;
    FontFamilyNode fontFamily;

    QHash<QLocale, QString> familyNames = {{QLocale(QLocale::English), name}};
    // TODO: can and should we translate this?
    QHash<QLocale, QString> styleNames = {{QLocale(QLocale::English), "Regular"}};

    fontFamily.fontFamily = name;
    fontFamily.localizedFontFamilies = familyNames;

    fontFamily.type = KoSvgText::OpenTypeFontType;

    typographicFamily = fontFamily;
    fontFamily.axes.insert(WEIGHT_TAG, KoSvgText::FontFamilyAxis::weightAxis(400));
    fontFamily.fontStyle = styleNames.values().first();
    fontFamily.localizedFontStyle = styleNames;
    QString tag = KoWritingSystemUtils::sampleTagForQLocale(QLocale(QLocale::English));
    fontFamily.sampleStrings.insert(tag, KoWritingSystemUtils::samples().key(tag));

    auto typographic = d->fontFamilyCollection.insert(d->fontFamilyCollection.childEnd(), typographicFamily);
    d->fontFamilyCollection.insert(childEnd(typographic), fontFamily);
}

KoFontFamilyWWSRepresentation createRepresentation(KisForest<FontFamilyNode>::child_iterator wws, KisForest<FontFamilyNode>::child_iterator typographic, bool singleFamily) {
    KoFontFamilyWWSRepresentation representation;
    representation.fontFamilyName = wws->fontFamily;
    representation.localizedFontFamilyNames = wws->localizedFontFamilies;
    if (!singleFamily)  {
        // This funnels the typographic family to the resource, so that resources may potentially be sorted by their typographic family.
        representation.typographicFamilyName = typographic->fontFamily;
        representation.localizedTypographicFamily = typographic->localizedFontFamilies;
        representation.localizedTypographicStyles = wws->localizedTypographicStyle;
    }
    representation.isVariable = wws->isVariable;
    representation.colorBitMap = wws->colorBitMap;
    representation.colorClrV0 = wws->colorClrV0;
    representation.colorClrV1 = wws->colorClrV1;
    representation.colorSVG = wws->colorSVG;
    representation.type = wws->type;

    for (auto subFamily = childBegin(wws); subFamily != childEnd(wws); subFamily++) {
        KoSvgText::FontFamilyStyleInfo style;
        if (subFamily == childBegin(wws)
                || representation.lastModified < subFamily->lastModified) {
            representation.lastModified = subFamily->lastModified;
        }
        representation.sampleStrings.insert(subFamily->sampleStrings);
        Q_FOREACH(const QLocale &locale, subFamily->supportedLanguages) {
            if (!representation.supportedLanguages.contains(locale)) {
                representation.supportedLanguages.append(locale);
            }
        }

        for (int a = 0; a < subFamily->axes.size(); a++) {
            QString key = subFamily->axes.keys().at(a);
            KoSvgText::FontFamilyAxis axis = subFamily->axes.value(key);
            KoSvgText::FontFamilyAxis mainAxis = representation.axes.value(key);
            mainAxis.min = qMin(mainAxis.min, axis.min);
            mainAxis.defaultValue = axis.defaultValue;
            mainAxis.max = qMax(mainAxis.max, axis.max);
            mainAxis.localizedLabels.insert(axis.localizedLabels);
            mainAxis.tag = axis.tag;
            mainAxis.axisHidden = axis.axisHidden;
            representation.axes.insert(mainAxis.tag, mainAxis);

            if (!subFamily->isVariable) {
                style.instanceCoords.insert(key, axis.value);
            }
        }
        if (!subFamily->isVariable) {
            if (!subFamily->localizedWWSStyle.isEmpty()) {
                style.localizedLabels = subFamily->localizedWWSStyle;
            } else if (!subFamily->localizedTypographicStyle.isEmpty()) {
                style.localizedLabels = subFamily->localizedTypographicStyle;
            } else if (!subFamily->localizedFontStyle.isEmpty()) {
                style.localizedLabels = subFamily->localizedFontStyle;
            } else {
                style.localizedLabels.insert(QLocale(QLocale::English), subFamily->fontStyle);
            }
            style.isItalic = subFamily->isItalic;
            style.isOblique = subFamily->isOblique;
            representation.styles.append(style);
        } else {
            for (int i = 0; i < subFamily->styleInfo.size(); i++) {
                KoSvgText::FontFamilyStyleInfo styleInfo = subFamily->styleInfo.at(i);
                if (!styleInfo.localizedLabels.isEmpty()) {
                    styleInfo.isItalic = subFamily->isItalic;
                    styleInfo.isOblique = subFamily->isOblique;
                    representation.styles.append(styleInfo);
                }
            }
        }
    }
    return representation;
}

QList<KoFontFamilyWWSRepresentation> KoFFWWSConverter::collectFamilies() const
{
    QList<KoFontFamilyWWSRepresentation> collection;
    for (auto typographic = d->fontFamilyCollection.childBegin(); typographic != d->fontFamilyCollection.childEnd(); typographic++) {
        auto counter = childBegin(typographic);
        counter++;
        bool singleFamily = childBegin(typographic) != childEnd(typographic) && std::next(childBegin(typographic)) == childEnd(typographic);

        for (auto wws = childBegin(typographic); wws != childEnd(typographic); wws++) {

            collection.append(createRepresentation(wws, typographic, singleFamily));
        }
    }
    return collection;
}

KisForest<FontFamilyNode>::composition_iterator searchNodes (KisForest<FontFamilyNode>::composition_iterator it, KisForest<FontFamilyNode>::composition_iterator endIt, const QString family) {
    QString familySimplified = family;
    QString familyLower = family.toLower();
    // Qt's fontdatabase would add the vendor in [] behind the font name, when there were duplicates,
    // though sometimes there was no such explanation, so we should check against that...
    if (family.endsWith("]") && family.contains("[")) {
        familySimplified = family.split("[", Qt::SkipEmptyParts).first().trimmed().toLower();
    }
    for (; it != endIt; it++) {
        if (it.state() == KisForestDetail::Enter) {
            continue;
        }

        if (childBegin(it) == childEnd(it)) {
            // For the lowest nodes, we only want to test the full name.
            QStringList local = it->localizedFullName.values();

            // For some fonts, the full name is the exact same as the typographic family name,
            // in which case, we want to ignore the full name.
            QStringList localFamily = it->localizedFontFamilies.values();
            bool inLocalFamilyToo = (localFamily.contains(familySimplified, Qt::CaseInsensitive)
                                  || localFamily.contains(familyLower, Qt::CaseInsensitive));

            if (local.contains(familySimplified, Qt::CaseInsensitive)
                    || local.contains(familyLower, Qt::CaseInsensitive)) {
                if (inLocalFamilyToo) continue;
                break;
            } else {
                continue;
            }
        }

        QStringList local = it->localizedFontFamilies.values();
        QString itFamilyLower = QString(it->fontFamily).toLower();
        if (itFamilyLower == familySimplified
                || itFamilyLower == familyLower
                || local.contains(familySimplified, Qt::CaseInsensitive)
                || local.contains(familyLower, Qt::CaseInsensitive)) {
            break;
        }

    }
    return it;
}

std::optional<KoFontFamilyWWSRepresentation> KoFFWWSConverter::representationByFamilyName(const QString &familyName) const
{
    for (auto typographic = d->fontFamilyCollection.childBegin(); typographic != d->fontFamilyCollection.childEnd(); typographic++) {
        auto counter = childBegin(typographic);
        counter++;
        bool singleFamily = counter == childEnd(typographic);

        for (auto wws = childBegin(typographic); wws != childEnd(typographic); wws++) {
            if (wws->fontFamily == familyName) {
                return std::make_optional(createRepresentation(wws, typographic, singleFamily));
            }
        }
    }
    return std::nullopt;
}

std::optional<QString> KoFFWWSConverter::wwsNameByFamilyName(const QString familyName) const
{
    auto it = d->fontFamilyCollection.compositionBegin();
    it = searchNodes(it, d->fontFamilyCollection.compositionEnd(), familyName);
    if (it != d->fontFamilyCollection.compositionEnd()) {

        bool isChild = childBegin(it) == childEnd(it);
        auto wws = siblingCurrent(it);

        // check if we're in a typographic family by testing the hierarchy.
        // if so, select the wws family.
        auto hierarchy = hierarchyBegin(wws);
        hierarchy++;
        if (isChild) {
            wws = siblingCurrent(hierarchy);
        } else if (hierarchy == hierarchyEnd(wws)) {
            wws = childBegin(it);
        }
        return std::make_optional(wws->fontFamily);
    }
    return std::nullopt;
}
#include <KoCssTextUtils.h>
QVector<FontFamilyNode> findNodesByAxis(const QVector<FontFamilyNode> &nodes, const QString axisTag, const qreal &value, const qreal &defaultValue, const qreal &defaultValueUpper) {
    QVector<FontFamilyNode> candidates;
    QVector<qreal> values;
    Q_FOREACH (const FontFamilyNode &node, nodes) {
        qreal selectingVal = defaultValue;

        if (node.axes.keys().contains(axisTag)) {
            KoSvgText::FontFamilyAxis axis = node.axes.value(axisTag);
            selectingVal = axis.value;
            if (axis.variableAxis) {
                if (value >= axis.min && value <= axis.max) {
                    candidates.append(node);
                    selectingVal = value;
                } else {
                    values.append(axis.min);
                    values.append(axis.max);
                }
                continue;
            }
            values.append(selectingVal);
        }
    }
    // We found some variable fonts already, so lets return early.
    if (!candidates.isEmpty()) {
        return candidates;
    }

    // follow the CSS Fonts selection mechanism.
    bool shouldNotReturnDefault = ((axisTag == ITALIC_TAG || axisTag == SLANT_TAG) && value != defaultValue);
    qreal selectedValue = KoCssTextUtils::cssSelectFontStyleValue(values, value, defaultValue, defaultValueUpper, shouldNotReturnDefault);

    Q_FOREACH (const FontFamilyNode &node, nodes) {
        if (node.axes.keys().contains(axisTag)) {
            KoSvgText::FontFamilyAxis axis = node.axes.value(axisTag);
            if (axis.value == selectedValue) {
                candidates.append(node);
            }
        } else if (value == defaultValue && !shouldNotReturnDefault) {
            candidates.append(node);
        }
    }
    return candidates;
}

QVector<KoFFWWSConverter::FontFileEntry> KoFFWWSConverter::candidatesForCssValues(const KoCSSFontInfo info,
                                                     quint32 xRes, quint32 yRes) const
{
    QVector<FontFileEntry> candidateFileNames;

    int pixelSize = info.size * (qMin(xRes, yRes) / 72.0);

    Q_FOREACH(const QString &family, info.families) {
        auto it = d->fontFamilyCollection.compositionBegin();
        it = searchNodes(it, d->fontFamilyCollection.compositionEnd(), family);
        if (it != d->fontFamilyCollection.compositionEnd()) {
            auto wws = siblingCurrent(it);

            // check if we're in a typographic family by testing the hierarchy.
            // if so, select all subnodes.
            // Because we test subtree depth-first for finding the nodes, wws
            // and full names are tested before typographic.
            auto hierarchy = hierarchyBegin(wws);
            hierarchy++;
            QVector<FontFamilyNode> candidates;
            if (hierarchy == hierarchyEnd(wws)) {
                auto nodes = subtreeBegin(wws);
                auto endNodes = subtreeEnd(wws);
                for (;nodes != endNodes; nodes++) {
                    if (childBegin(nodes) == childEnd(nodes)) {
                        candidates.append(*nodes);
                    }
                }
            } else if (childBegin(wws) == childEnd(wws)) {
                candidates.append(*wws);
            } else {
                auto style = childBegin(wws);
                auto styleEnd = childEnd(wws);
                for (;style != styleEnd; style++) {
                    candidates.append(*style);
                }
            }

            if (candidates.size() > 1) {
                // first find width
                candidates = findNodesByAxis(candidates, WIDTH_TAG, info.width, 100.0, 100.0);
            }

            if (candidates.size() > 1) {
                // then find weight
                candidates = findNodesByAxis(candidates, WEIGHT_TAG, info.weight, 400.0, 500.0);
            }
            // then match italic
            if (candidates.size() > 1) {
                QVector<FontFamilyNode> italics;
                QVector<FontFamilyNode> obliques;

                if (wws->isVariable) {
                    qreal slantValue = info.slantMode == QFont::StyleItalic? 11: info.autoSlant? 14: info.slantValue;
                    italics = findNodesByAxis(candidates, ITALIC_TAG, 1.0, 0.0, 0.0);
                    obliques = findNodesByAxis(candidates, SLANT_TAG, -slantValue, 0.0, 0.0);
                }
                if (italics.isEmpty() && obliques.isEmpty()) {
                    Q_FOREACH(const FontFamilyNode &node, candidates) {
                        if (node.isItalic) {
                            if (!node.isOblique) {
                                italics.append(node);
                            } else {
                                obliques.append(node);
                            }
                        }
                    }
                }

                if (info.slantMode == QFont::StyleItalic) {
                    if (!italics.isEmpty()) {
                        candidates = italics;
                    } else if (!obliques.isEmpty()) {
                        candidates = obliques;
                    }
                } else if (info.slantMode == QFont::StyleOblique) {
                    if (!obliques.isEmpty()) {
                        candidates = obliques;
                    } else if (!italics.isEmpty()) {
                        candidates = italics;
                    }
                } else {
                    QStringList slantedFontFiles;
                    QVector<FontFamilyNode> regular;
                    Q_FOREACH(const FontFamilyNode &italic, italics) {
                        slantedFontFiles.append(italic.fileName);
                    }
                    Q_FOREACH(const FontFamilyNode &oblique, obliques) {
                        slantedFontFiles.append(oblique.fileName);
                    }
                    Q_FOREACH(const FontFamilyNode &node, candidates) {
                        if (!slantedFontFiles.contains(node.fileName)) {
                            regular.append(node);
                        }
                    }
                    if (!regular.isEmpty()) {
                        candidates = regular;
                    }
                }
            }

            // prefer opentype
            if (candidates.size() > 1) {
                QVector<FontFamilyNode> openType;
                Q_FOREACH(const FontFamilyNode &node, candidates) {
                    if (node.type == KoSvgText::OpenTypeFontType) {
                        openType.append(node);
                    }
                }
                if (!openType.isEmpty()) {
                    candidates = openType;
                }
            }

            // finally, match size.
            Q_FOREACH(const FontFamilyNode &node, candidates) {
                QStringList fileNames = node.otherFiles;
                fileNames.append(node.fileName);
                fileNames = node.pixelSizes.value(pixelSize, fileNames);
                Q_FOREACH(const QString &fileName, fileNames) {
                    if (fileName.isEmpty()) continue;
                    FontFileEntry entry;
                    entry.fileName = fileName;
                    entry.fontIndex = node.fileIndex;
                    candidateFileNames.append(entry);
                }
            }
        }
    }
    return candidateFileNames;
}

void KoFFWWSConverter::debugInfo() const
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

