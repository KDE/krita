/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFFWWSConverter.h"

#include <KisForest.h>
#include <KisStaticInitializer.h>
#include <hb.h>
#include <hb-ft.h>
#include FT_TRUETYPE_TABLES_H

#include <QFileInfo>

#include <QFontDatabase>



struct FontFamilySizeInfo {
    bool isSet = false;
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
            return low == other.low && high == other.high;
        } else {
            return designSize == other.designSize;
        }
    }
};

struct FontFamilyNode {

    FontFamilyNode() {}

    QString fontFamily;
    QString fontStyle;
    QString fileName;
    int fileIndex = 0;

    QHash<QLocale::Script, QString> sampleStrings; /// sample string used to generate the preview;
    QList<QLocale> supportedLanguages;

    QStringList otherFiles;

    QHash<QLocale, QString> localizedFontFamilies;
    QHash<QLocale, QString> localizedFontStyle;
    QHash<QLocale, QString> localizedTypographicStyle;
    QHash<QLocale, QString> localizedWWSStyle;

    QHash<QString, KoSvgText::FontFamilyAxis> axes;
    QList<KoSvgText::FontFamilyStyleInfo> styleInfo;

    QHash<int, QStringList> pixelSizes;
    FontFamilySizeInfo sizeInfo;

    bool isItalic = false;
    bool isOblique = false;

    bool compareAxes(QHash<QString, KoSvgText::FontFamilyAxis> otherAxes) {
        if (axes.keys() != otherAxes.keys()) {
            return false;
        }
        for (int k = 0; k < axes.keys().size(); k++) {
            KoSvgText::FontFamilyAxis a = axes.value(axes.keys().at(k));
            KoSvgText::FontFamilyAxis b = otherAxes.value(axes.keys().at(k));
            if (a.value != b.value) {
                return false;
            }
        }
        return true;
    }

    static FontFamilyNode createWWSFam(const FontFamilyNode &child, QStringList existingWWSNames) {
        FontFamilyNode wwsFamily;
        if (child.type != KoSvgText::OpenTypeFontType) {
            if (child.fontStyle.toLower() == "regular") {
                wwsFamily.fontFamily = child.fontFamily;
            } else {
                wwsFamily.fontFamily = child.fontFamily + " " + child.fontStyle;
            }
            wwsFamily.fontStyle = child.fontStyle;
        } else {
            wwsFamily.fontFamily = child.fontFamily;
            if (existingWWSNames.contains(child.fontFamily)) {
                wwsFamily.fontFamily = child.fontFamily + " " + child.fontStyle;
            }
        }
        wwsFamily.localizedTypographicStyle = child.localizedTypographicStyle;
        wwsFamily.localizedFontFamilies = child.localizedFontFamilies;
        wwsFamily.isVariable = child.isVariable;
        wwsFamily.colorBitMap = child.colorBitMap;
        wwsFamily.colorSVG = child.colorSVG;
        wwsFamily.colorClrV0 = child.colorClrV0;
        wwsFamily.colorClrV1 = child.colorClrV1;
        return wwsFamily;
    }

    KoSvgText::FontFormatType type = KoSvgText::UnknownFontType;
    bool isVariable = false;
    bool colorClrV0 = false;
    bool colorClrV1 = false;
    bool colorSVG = false;
    bool colorBitMap = false;

    QStringList debugInfo();
};

QStringList FontFamilyNode::debugInfo()
{
    QString style = isItalic? isOblique? "Oblique": "Italic": "Roman";
    QStringList debug = {QString("\'%1\' \'%2\', style: %3, type:%4").arg(fontFamily, fontStyle, style).arg(type)};
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

struct KoFFWWSConverter::Private {
    Private() {}

    KisForest<FontFamilyNode> fontFamilyCollection;
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
        if (!languages.isEmpty()) {
            addSupportedLanguagesByFile(filename, indexValue, languages);
        }
    }
    return success;
}

bool KoFFWWSConverter::addFontFromFile(const QString &filename, const int index, FT_LibrarySP freeTypeLibrary) {

    FontFamilyNode fontFamily;
    fontFamily.fileName = filename;
    fontFamily.fileIndex = index;
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
    FontFamilyNode typographicFamily;
    FontFamilyNode wwsFamily;
    bool isWWSFamilyWithoutName = false;

    if (!FT_IS_SFNT(face.data())) {
        fontFamily.type = FT_IS_SCALABLE(face.data())? KoSvgText::Type1FontType: KoSvgText::BDFFontType;

        fontFamily.isItalic = face->style_flags & FT_STYLE_FLAG_ITALIC;
        if (face->style_flags & FT_STYLE_FLAG_BOLD) {
            fontFamily.axes.insert("wght", KoSvgText::FontFamilyAxis::weightAxis(700));
        } else {
            fontFamily.axes.insert("wght", KoSvgText::FontFamilyAxis::weightAxis(400));
        }
        if (fontFamily.fontFamily.isEmpty()) {
            fontFamily.fontFamily = QFileInfo(fontFamily.fileName).baseName();
        }
        for (int i=0; i< face->num_fixed_sizes; i++) {
            // 64 = Freetype pixel
            fontFamily.pixelSizes.insert((face->available_sizes[i].size / 64.0), {fontFamily.fileName});
        }
    } else {
        fontFamily.type = KoSvgText::OpenTypeFontType;
        hb_face_t *hbFace = hb_ft_face_create_referenced(face.data());

        // Retrieve width, weight and slant data.
        TT_OS2 *os2Table = nullptr;
        os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
        if (os2Table) {
            fontFamily.axes.insert("wght", KoSvgText::FontFamilyAxis::weightAxis(os2Table->usWeightClass));
            fontFamily.axes.insert("wdth", KoSvgText::FontFamilyAxis::widthAxis(percentageFromUsWidthClass(os2Table->usWidthClass)));
            if (os2Table->fsSelection & OS2_BOLD && os2Table->usWeightClass == 400) {
                fontFamily.axes.insert("wght", KoSvgText::FontFamilyAxis::weightAxis(700));
            }
            fontFamily.isItalic = (os2Table->fsSelection & OS2_ITALIC);
            fontFamily.isOblique = (os2Table->fsSelection & OS2_OBLIQUE);
            if (os2Table->fsSelection & OS2_REGULAR) {
                fontFamily.axes.insert("wght", KoSvgText::FontFamilyAxis::weightAxis(os2Table->usWeightClass));
                fontFamily.isItalic = false;
                fontFamily.isOblique = false;
            }
            if (os2Table->version >= 5) {
                FontFamilySizeInfo sizeInfo;
                qreal twip = 0.05; ///< twip is 'Twenty-in-point';
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
        if (hb_ot_layout_get_size_params(hbFace, &designSize, &subFamilyId, &sizeNameId, &rangeStart, &rangeEnd)) {
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
        if (hb_ot_var_has_data(hbFace)) {
            fontFamily.isVariable = true;
            uint count = hb_ot_var_get_axis_count(hbFace);
            uint maxInfos = 1;
            QStringList axesTags;
            for (uint i = 0; i < count; i++) {
                KoSvgText::FontFamilyAxis axisInfo;
                hb_ot_var_axis_info_t axis;
                hb_ot_var_get_axis_infos(hbFace, i, &maxInfos, &axis);
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
            count = hb_ot_var_get_named_instance_count (hbFace);
            for (uint i = 0; i < count; i++) {
                QHash<QString, float> instanceCoords;
                uint coordLength = axesTags.size();
                float coordinate[coordLength];
                hb_ot_var_named_instance_get_design_coords (hbFace, i, &coordLength, coordinate);
                for (uint j =0; j < coordLength; j++ ){
                    instanceCoords.insert(axesTags.value(j), coordinate[j]);
                }
                KoSvgText::FontFamilyStyleInfo style;
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
        wwsFamily.colorBitMap = fontFamily.colorBitMap;
        wwsFamily.colorSVG = fontFamily.colorSVG;
        wwsFamily.colorClrV0 = fontFamily.colorClrV0;

        uint numEntries = 0;
        const hb_ot_name_entry_t *entries = hb_ot_name_list_names(hbFace, &numEntries);

        QHash<QLocale, QString> ribbiFamilyNames;
        QHash<QLocale, QString> ribbiStyleNames;
        QHash<QLocale, QString> WWSFamilyNames;
        QHash<QLocale, QString> WWSStyleNames;
        QHash<QLocale, QString> typographicFamilyNames;
        QHash<QLocale, QString> typographicStyleNames;
        for (uint i = 0; i < numEntries; i++) {
            hb_ot_name_entry_t entry = entries[i];
            QString lang(hb_language_to_string(entry.language));
            QLocale locale(lang);
            uint length = hb_ot_name_get_utf8(hbFace, entry.name_id, entry.language, nullptr, nullptr)+1;
            char buff[length];
            hb_ot_name_get_utf8(hbFace, entry.name_id, entry.language, &length, buff);
            QString name = QString::fromUtf8(buff, length);

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
        if (!ribbiStyleNames.isEmpty()) {
            fontFamily.fontStyle = ribbiStyleNames.value(english, ribbiStyleNames.values().first());
            fontFamily.localizedFontStyle = ribbiStyleNames;
        }
        if (!WWSFamilyNames.isEmpty()) {
            wwsFamily.fontFamily = WWSFamilyNames.value(english, WWSFamilyNames.values().first());
            wwsFamily.localizedFontFamilies = WWSFamilyNames;
        }
        fontFamily.localizedWWSStyle = WWSStyleNames;

        hb_face_destroy(hbFace);
    }
    if (typographicFamily.fontFamily.isEmpty()) {
        typographicFamily.fontFamily = fontFamily.fontFamily;
    }
    wwsFamily.isVariable = fontFamily.isVariable;

    //qDebug() << "adding..." << fontFamily.fontFamily << fontFamily.fileName;

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

void KoFFWWSConverter::addSupportedLanguagesByFile(const QString &filename, const int index, const QList<QLocale> &supportedLanguages)
{
    auto it = d->fontFamilyCollection.depthFirstTailBegin();
    for (; it!= d->fontFamilyCollection.depthFirstTailEnd(); it++) {
        if (it->fileName == filename && it->fileIndex == index) {
            break;
        }
    }
    if (it != d->fontFamilyCollection.depthFirstTailEnd()) {
        it->supportedLanguages = supportedLanguages;
        Q_FOREACH(const QLocale &locale, supportedLanguages) {
            QLocale::Script script = locale.script();
            QString sample;

            if (!it->sampleStrings.keys().contains(script)) {

                // The QFontDatabase writing system list seems similar to the MacOS list.
                switch(script) {
                case QLocale::LatinScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Latin);
                    break;
                case QLocale::GreekScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Greek);
                    break;
                case QLocale::CyrillicScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Cyrillic);
                    break;
                case QLocale::ArmenianScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Armenian);
                    break;
                case QLocale::HebrewScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Hebrew);
                    break;
                case QLocale::ArabicScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Arabic);
                    break;
                case QLocale::SyriacScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Syriac);
                    break;
                case QLocale::ThaanaScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Thaana);
                    break;
                case QLocale::DevanagariScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Devanagari);
                    break;
                case QLocale::BengaliScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Bengali);
                    break;
                case QLocale::GurmukhiScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Gurmukhi);
                    break;
                case QLocale::GujaratiScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Gujarati);
                    break;
                case QLocale::OriyaScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Oriya);
                    break;
                case QLocale::TamilScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Tamil);
                    break;
                case QLocale::TeluguScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Telugu);
                    break;
                case QLocale::KannadaScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Kannada);
                    break;
                case QLocale::MalayalamScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Malayalam);
                    break;
                case QLocale::SinhalaScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Sinhala);
                    break;
                case QLocale::ThaiScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Thai);
                    break;
                case QLocale::LaoScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Lao);
                    break;
                case QLocale::TibetanScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Tibetan);
                    break;
                case QLocale::MyanmarScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Myanmar);
                    break;
                case QLocale::GeorgianScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Georgian);
                    break;
                case QLocale::KhmerScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Khmer);
                    break;
                case QLocale::SimplifiedChineseScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::SimplifiedChinese);
                    break;
                case QLocale::TraditionalChineseScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::TraditionalChinese);
                    break;
                case QLocale::JapaneseScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Japanese);
                    break;
                case QLocale::KoreanScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Korean);
                    break;
                case QLocale::OghamScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Ogham);
                    break;
                case QLocale::RunicScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Runic);
                    break;
                case QLocale::NkoScript:
                    sample = QFontDatabase::writingSystemSample(QFontDatabase::Nko);
                    break;
                default:
                    // TODO: misses symbol and vietnamese sample, but I have no idea what is meant by the latter...
                    sample = QString();
                }
                if (!sample.isEmpty()) {
                    it->sampleStrings.insert(script, sample);
                }
            }
        }

    }
}

void KoFFWWSConverter::sortIntoWWSFamilies()
{
    QStringList wwsNames;
    for (auto typographic = d->fontFamilyCollection.childBegin(); typographic != d->fontFamilyCollection.childEnd(); typographic++) {
        KisForest<FontFamilyNode> tempList;

        QVector<qreal> weights;
        QVector<qreal> widths;
        QVector<KoSvgText::FontFormatType> types;

        for (auto child = childBegin(typographic); child != childEnd(typographic); child++) {
            if (childBegin(child) != childEnd(child)) {
                wwsNames.append(child->fontFamily);
                continue;
            }
            tempList.insert(tempList.childEnd(), *child);
            qreal wght = child->axes.value("wght", KoSvgText::FontFamilyAxis::weightAxis(400)).value;
            if (!weights.contains(wght)) weights.append(wght);
            qreal wdth = child->axes.value("wdth", KoSvgText::FontFamilyAxis::widthAxis(100)).value;
            if (!widths.contains(wdth)) widths.append(wdth);
            types.append(child->type);
            d->fontFamilyCollection.erase(child);
        }
        //Do most regular first...
        if (KisForestDetail::size(tempList) > 0) {
            KoSvgText::FontFormatType testType = types.contains(KoSvgText::OpenTypeFontType)? KoSvgText::OpenTypeFontType: types.first();
            QVector<QPair<QString, QString>> existing;
            for (auto font = tempList.childBegin(); font != tempList.childEnd(); font++) {
                qreal testWeight = weights.contains(400)? 400: weights.first();
                qreal testWidth = widths.contains(100)? 100: widths.first();

                bool widthTested = !font->axes.keys().contains("wdth");
                widthTested = widthTested? true: font->axes.value("wdth").value == testWidth;

                QPair<QString, QString> fontStyle(font->fontFamily, font->fontStyle);
                if (font->axes.value("wght").value == testWeight && widthTested
                        && !font->isItalic
                        && !font->isOblique
                        && font->type == testType
                        && !existing.contains(fontStyle)) {
                    FontFamilyNode wwsFamily = FontFamilyNode::createWWSFam(*font, wwsNames);
                    wwsNames.append(wwsFamily.fontFamily);
                    existing.append(fontStyle);

                    auto newWWS = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                    d->fontFamilyCollection.insert(childEnd(newWWS), *font);
                    tempList.erase(font);
                }
            }
            for (auto font = tempList.childBegin(); font != tempList.childEnd(); font++) {
                auto wws = childBegin(typographic);
                for (; wws != childEnd(typographic); wws++) {
                    auto wwsChild = childBegin(wws);
                    if (font->type != KoSvgText::OpenTypeFontType && font->type != KoSvgText::Type1FontType) {
                        // Hack for really old fonts.
                        // It's questionable whether this is wise, given that it is not the family name,
                        // but it seems CSS is explicitely vague about what the family name is, because
                        // of the varying ways a font can be assigned a family name.
                        // Like, it is technically correct, because it allows us to select fonts with CSS
                        // that would otherwise be unselectable, but this does mean that other applications
                        // would need to have our exact idea of a CSS family name.
                        if (wws->fontStyle.toLower() != "regular"
                                && !font->fontStyle.contains(wws->fontStyle)) {
                            continue;
                        }
                    } else if (font->isVariable != wwsChild->isVariable) {
                        // try not to mix variable and non-variable fonts into the same family.
                        continue;
                    }

                    if (!wwsChild->sizeInfo.compare(font->sizeInfo)) {
                        // Skip sorting if the WWS family has size info that is incompatible with the sorted font.
                        continue;
                    }
                    for (; wwsChild != childEnd(wws); wwsChild++) {
                        if (wwsChild->isItalic == font->isItalic
                                && wwsChild->isOblique == font->isOblique
                                && wwsChild->compareAxes(font->axes)) {
                            // TODO: test color.
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
                            break;
                        } else {
                            continue;
                        }
                    } else {
                        d->fontFamilyCollection.insert(childEnd(wws), *font);
                        break;
                    }
                }
                if (wws == childEnd(typographic)) {
                    FontFamilyNode wwsFamily = FontFamilyNode::createWWSFam(*font, wwsNames);
                    wwsNames.append(wwsFamily.fontFamily);
                    auto newWWS = d->fontFamilyCollection.insert(childEnd(typographic), wwsFamily);
                    d->fontFamilyCollection.insert(childEnd(newWWS), *font);
                }
            }
        }

    }
}

KoFontFamilyWWSRepresentation createRepresentation(KisForest<FontFamilyNode>::child_iterator wws, KisForest<FontFamilyNode>::child_iterator typographic, bool singleFamily) {
    KoFontFamilyWWSRepresentation representation;
    representation.fontFamilyName = wws->fontFamily;
    representation.localizedFontFamilyNames = wws->localizedFontFamilies;
    if (!singleFamily)  {
        representation.typographicFamilyName = typographic->fontFamily;
        representation.localizedTypographicFamily = typographic->localizedFontFamilies;
        representation.localizedTypographicStyles = wws->localizedTypographicStyle;
    }
    representation.isVariable = wws->isVariable;
    representation.colorBitMap = wws->colorBitMap;
    representation.colorClrV0 = wws->colorClrV0;
    representation.colorClrV1 = wws->colorClrV1;
    representation.colorSVG = wws->colorSVG;

    for (auto subFamily = childBegin(wws); subFamily != childEnd(wws); subFamily++) {
        KoSvgText::FontFamilyStyleInfo style;
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
        bool singleFamily = counter == childEnd(typographic);

        for (auto wws = childBegin(typographic); wws != childEnd(typographic); wws++) {

            collection.append(createRepresentation(wws, typographic, singleFamily));
        }
    }
    return collection;
}

KoFontFamilyWWSRepresentation KoFFWWSConverter::representationByFamilyName(const QString &familyName, bool *found) const
{
    KoFontFamilyWWSRepresentation representation;
    if (found) {
        *found = false;
    }
    for (auto typographic = d->fontFamilyCollection.childBegin(); typographic != d->fontFamilyCollection.childEnd(); typographic++) {
        auto counter = childBegin(typographic);
        counter++;
        bool singleFamily = counter == childEnd(typographic);

        for (auto wws = childBegin(typographic); wws != childEnd(typographic); wws++) {
            if (wws->fontFamily == familyName) {
                representation = createRepresentation(wws, typographic, singleFamily);
                if (found) {
                    *found = true;
                }
                return representation;
            }
        }
    }
    return representation;
}

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
    std::sort(values.begin(), values.end());
    qreal selectedValue = defaultValue;
    auto upper = std::lower_bound(values.begin(), values.end(), value);
    if (upper == values.end()) {
        upper--;
    }
    auto lower = upper;
    if (lower != values.begin() && *lower > value) {
        lower--;
    }

    // ... Which wants to select the lower possible selection when the value is below the default.
    if (value < defaultValue) {
        selectedValue = *lower;
    // ... the higher closest value when the value is higher than the default (upper bound)
    } else if (value > defaultValueUpper) {
        selectedValue = *upper;
    } else {
        // ... and if the value is between the lower and upper default bounds, first higher (within bounds)
        // then lower, then higher.
        if (*upper <= defaultValueUpper && *lower != value) {
            selectedValue = *upper;
        } else {
            selectedValue = *lower;
        }
    }

    Q_FOREACH (const FontFamilyNode &node, nodes) {
        if (node.axes.keys().contains(axisTag)) {
            KoSvgText::FontFamilyAxis axis = node.axes.value(axisTag);
            if (axis.value == selectedValue) {
                candidates.append(node);
            }
        } else if (value == defaultValue) {
            candidates.append(node);
        }
    }
    return candidates;
}

QStringList KoFFWWSConverter::candidatesForCssValues(const QStringList &families,
                                                     const QMap<QString, qreal> &axisSettings,
                                                     quint32 xRes, quint32 yRes,
                                                     qreal size,
                                                     int weight, int width,
                                                     int slantMode, int slantValue) const
{
    Q_UNUSED(axisSettings)
    Q_UNUSED(slantValue)
    QStringList candidateFileNames;

    int pixelSize = size * (qMin(xRes, yRes) / 72.0);

    Q_FOREACH(const QString &family, families) {

        auto it = d->fontFamilyCollection.depthFirstTailBegin();
        for (; it != d->fontFamilyCollection.depthFirstTailEnd(); it++) {
            if (childBegin(it) == childEnd(it)) {
                // TODO: test full names (?);
                continue;
            }
            QStringList local = it->localizedFontFamilies.values();
            if (QString(it->fontFamily).toLower() == family.toLower()
                    || local.contains(family.toLower(), Qt::CaseInsensitive)) {
                break;
            }

        }
        if (it != d->fontFamilyCollection.depthFirstTailEnd()) {
            auto wws = siblingCurrent(it); //TODO: check if we're inside a wws or typographic family.
            auto style = childBegin(wws);
            auto styleEnd = childEnd(wws);
            if (style == styleEnd) {
                // direct match.
                QStringList fileNames = wws->pixelSizes.value(pixelSize, {wws->fileName});
                Q_FOREACH(const QString &fileName, fileNames) {
                    candidateFileNames.append(fileName);
                }

            } else {
                QVector<FontFamilyNode> candidates;
                for (;style != styleEnd; style++) {
                    candidates.append(*style);
                }

                if (candidates.size() > 1) {
                    // first find width
                    candidates = findNodesByAxis(candidates, "wdth", width, 100.0, 100.0);
                }

                if (candidates.size() > 1) {
                    // then find weight
                    candidates = findNodesByAxis(candidates, "wght", weight, 400.0, 500.0);

                }
                // then match italic
                if (candidates.size() > 1) {
                    QVector<FontFamilyNode> italics;
                    QVector<FontFamilyNode> obliques;
                    if (wws->isVariable) {
                        italics = findNodesByAxis(candidates, "ital", 1.0, 0.0, 0.0);
                        obliques = findNodesByAxis(candidates, "slnt", -slantValue, 0.0, 0.0);
                    }
                    if (italics.isEmpty() && obliques.isEmpty()) {
                        Q_FOREACH(const FontFamilyNode &node, candidates) {
                            if (slantMode > 0 && node.isItalic) {
                                if (!node.isOblique) {
                                    italics.append(node);
                                } else {
                                    obliques.append(node);
                                }
                            }
                        }
                    }
                    if (slantMode == QFont::StyleItalic) {
                        if (!italics.isEmpty()) {
                            candidates = italics;
                        } else if (!obliques.isEmpty()) {
                            candidates = obliques;
                        }
                    } else if (slantMode == QFont::StyleOblique) {
                        if (!obliques.isEmpty()) {
                            candidates = obliques;
                        } else if (!italics.isEmpty()) {
                            candidates = italics;
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
                        candidateFileNames.append(fileName);
                    }
                }
            }
        }
    }
    // TODO: There's something weird about not returning the file index as well, but am somewhat lost how to use it.
    return candidateFileNames;
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


