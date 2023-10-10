/*
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontRegistry.h"
#include "FlakeDebug.h"
#include "KoCssTextUtils.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGlobalStatic>
#include <QThread>
#include <QThreadStorage>
#include <QtGlobal>
#include <utility>

#include <optional>

#include <KoResourcePaths.h>
#include <kis_debug.h>

#include "KoFontLibraryResourceUtils.h"
#include "KoFFWWSConverter.h"
#include "KoFontChangeTracker.h"
#include "KoWritingSystemUtils.h"
#include <KisResourceLocator.h>
#include FT_TRUETYPE_TABLES_H
#include FT_FREETYPE_H


static unsigned int firstCharUcs4(const QStringView qsv)
{
    if (Q_UNLIKELY(qsv.isEmpty())) {
        return 0;
    }
    const QChar high = qsv.first();
    if (Q_LIKELY(!high.isSurrogate())) {
        return high.unicode();
    }
    if (Q_LIKELY(high.isHighSurrogate() && qsv.length() >= 2)) {
        const QChar low = qsv[1];
        if (Q_LIKELY(low.isLowSurrogate())) {
            return QChar::surrogateToUcs4(high, low);
        }
    }
    return QChar::ReplacementCharacter;
}

Q_GLOBAL_STATIC(KoFontRegistry, s_instance)

class Q_DECL_HIDDEN KoFontRegistry::Private
{
private:
    FcConfigSP m_config;
    QSharedPointer<KoFFWWSConverter> fontFamilyConverter;
    QSharedPointer<KoFontChangeTracker> changeTracker;

    struct ThreadData {
        FT_LibrarySP m_library;
        QHash<QString, FcPatternSP> m_patterns;
        QHash<QString, FcFontSetSP> m_fontSets;
        QHash<QString, FT_FaceSP> m_faces;
        QHash<QString, QVector<KoFFWWSConverter::FontFileEntry>> m_suggestedFiles;
        QHash<QString, KoSvgText::FontMetrics> m_fontMetrics;

        ThreadData(FT_LibrarySP lib)
            : m_library(std::move(lib))
        {
        }
    };

    QThreadStorage<QSharedPointer<ThreadData>> m_data;

    void initialize()
    {
        if (!m_data.hasLocalData()) {
            FT_Library lib = nullptr;
            FT_Error error = FT_Init_FreeType(&lib);
            if (error) {
                errorFlake << "Error with initializing FreeType library:" << error << "Current thread:" << QThread::currentThread()
                           << "GUI thread:" << qApp->thread();
            } else {
                m_data.setLocalData(QSharedPointer<ThreadData>::create(lib));
            }
        }
    }

public:
    Private()
    {
        FcConfig *config = FcConfigCreate();
        KIS_ASSERT(config && "No Fontconfig support available");

        /**
         * This loads the fontconfig configured on the host Linux system.
         *
         * Theoretically, it can cause some version inconsistency problems if
         * the version of fontconfig on the host differs from the version
         * of fontconfig shipped with Krita. But we estimate such risks as
         * negligible.
         */
        if (qgetenv("FONTCONFIG_PATH").isEmpty()) {
            QDir appdir("/etc/fonts");
            if (QFile::exists(appdir.absoluteFilePath("fonts.conf"))) {
                qputenv("FONTCONFIG_PATH", QFile::encodeName(QDir::toNativeSeparators(appdir.absolutePath())));
            } else {
                // Otherwise use default, which is defined in src/fcinit.c , windows and macos
                // default locations *are* defined in fontconfig's meson build system.
                appdir = QDir(KoResourcePaths::getApplicationRoot() +"/etc/fonts");
                if (QFile::exists(appdir.absoluteFilePath("fonts.conf"))) {
                    qputenv("FONTCONFIG_PATH", QFile::encodeName(QDir::toNativeSeparators(appdir.absolutePath())));
                }
            }
        }
        if (!FcConfigParseAndLoad(config, nullptr, FcTrue)) {
            errorFlake << "Failed loading the Fontconfig configuration";
            config = FcConfigGetCurrent();
        } else {
            FcConfigSetCurrent(config);
        }
        m_config.reset(config);

        // Add fonts folder from resource folder.
        const QString fontsFolder = KoResourcePaths::saveLocation("data", "/fonts/", true);
        FcConfigAppFontAddDir(m_config.data(), reinterpret_cast<const FcChar8 *>(fontsFolder.toUtf8().data()));

        /// Setup the change tracker.
        FcStrList *list = FcConfigGetFontDirs(m_config.data());
        FcStrListFirst(list);
        FcChar8 *dirString = FcStrListNext(list);
        QString path = QString::fromUtf8(reinterpret_cast<char *>(dirString));
        QStringList paths;
        while (!path.isEmpty()) {
            paths.append(path);
            FcStrListNext(list);
            dirString = FcStrListNext(list);
            path = QString::fromUtf8(reinterpret_cast<char *>(dirString));
        }
        paths.append(fontsFolder);
        FcStrListDone(list);
        changeTracker.reset(new KoFontChangeTracker(paths));
        changeTracker->resetChangeTracker();

        reloadConverter();
    }

    ~Private() = default;

    FT_LibrarySP library()
    {
        if (!m_data.hasLocalData())
            initialize();
        return m_data.localData()->m_library;
    }

    QHash<QString, FcPatternSP> &patterns()
    {
        if (!m_data.hasLocalData())
            initialize();
        return m_data.localData()->m_patterns;
    }

    QHash<QString, FcFontSetSP> &sets()
    {
        if (!m_data.hasLocalData())
            initialize();
        return m_data.localData()->m_fontSets;
    }

    QHash<QString, FT_FaceSP> &typeFaces()
    {
        if (!m_data.hasLocalData())
            initialize();
        return m_data.localData()->m_faces;
    }

    FcConfigSP config() const
    {
        return m_config;
    }

    QSharedPointer<KoFFWWSConverter> converter() const {
        return fontFamilyConverter;
    }

    QSharedPointer<KoFontChangeTracker> fontChangeTracker() const {
        return changeTracker;
    }

    bool reloadConverter() {
        fontFamilyConverter.reset(new KoFFWWSConverter());
        FcObjectSet *objectSet = FcObjectSetBuild(FC_FAMILY, FC_FILE, FC_INDEX, FC_LANG, FC_CHARSET, nullptr);
        FcFontSetSP allFonts(FcFontList(m_config.data(), FcPatternCreate(), objectSet));

        for (int j = 0; j < allFonts->nfont; j++) {
            fontFamilyConverter->addFontFromPattern(allFonts->fonts[j], library());
        }
        fontFamilyConverter->addGenericFamily("serif");
        fontFamilyConverter->addGenericFamily("sans-serif");
        fontFamilyConverter->addGenericFamily("monospace");
        fontFamilyConverter->sortIntoWWSFamilies();
        return true;
    }

    QHash<QString, QVector<KoFFWWSConverter::FontFileEntry>> &suggestedFileNames()
    {
        if (!m_data.hasLocalData())
            initialize();
        return m_data.localData()->m_suggestedFiles;
    }

    QHash<QString, KoSvgText::FontMetrics> &fontMetrics() {
        if (!m_data.hasLocalData())
            initialize();
        return m_data.localData()->m_fontMetrics;
    }

    void debugConverter() {
        fontFamilyConverter->debugInfo();
    }

    void updateConfig() {
        if (FcConfigBuildFonts(m_config.data())) {
            reloadConverter();
            KisResourceLocator::instance()->updateFontStorage();
            changeTracker->resetChangeTracker();
        }
    }
};

KoFontRegistry::KoFontRegistry(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
    connect(d->fontChangeTracker().data(), SIGNAL(sigUpdateConfig()), this, SLOT(updateConfig()), Qt::UniqueConnection);
}

KoFontRegistry::~KoFontRegistry() {
}

KoFontRegistry *KoFontRegistry::instance()
{
    return s_instance;
}

QString modificationsString(KoCSSFontInfo info, quint32 xRes, quint32 yRes) {
    QString modifications;
    if (info.size > -1) {
        modifications += QString::number(info.size) + ":" + QString::number(xRes) + "x" + QString::number(yRes);
    }
    if (info.fontSizeAdjust != 1.0) {
        modifications += QString::number(info.fontSizeAdjust);
    }
    if (info.weight != 400) {
        modifications += "weight:"+QString::number(info.weight);
    }
    if (info.width != 100) {
        modifications += "width:"+QString::number(info.width);
    }
    if (info.slantMode != 0) {
        modifications += "slantMode:"+QString::number(info.slantMode);
        if (info.slantValue != 0) {
            modifications += "val:"+QString::number(info.slantValue);
        }
    }
    if (!info.axisSettings.isEmpty()) {
        Q_FOREACH (const QString &key, info.axisSettings.keys()) {
            modifications += "|" + key + QString::number(info.axisSettings.value(key));
        }
    }
    return modifications;
}

std::optional<KoFFWWSConverter::FontFileEntry> getFontFileEntry(const FcPattern *p) {

    FcChar8 *fileValue{};
    if (FcPatternGetString(p, FC_FILE, 0, &fileValue) != FcResultMatch) {
        debugFlake << "Failed to get font file for" << p;
        return {};
    }
    const QString fontFileName = QString::fromUtf8(reinterpret_cast<char *>(fileValue));

    int indexValue{};
    if (FcPatternGetInteger(p, FC_INDEX, 0, &indexValue) != FcResultMatch) {
        debugFlake << "Failed to get font index for" << p << "(file:" << fontFileName << ")";
        return {};
    }

    return {{fontFileName, indexValue}};
}

std::vector<FT_FaceSP> KoFontRegistry::facesForCSSValues(QVector<int> &lengths,
                                                         KoCSSFontInfo info,
                                                         const QString &text,
                                                         quint32 xRes,
                                                         quint32 yRes, bool disableFontMatching,
                                                         const QString &language)
{
    QString modifications = modificationsString(info, xRes, yRes);

    QVector<KoFFWWSConverter::FontFileEntry> candidates;
    const QString suggestedHash = info.families.join("+") + ":" + modifications;
    auto entry = d->suggestedFileNames().find(suggestedHash);
    if (entry != d->suggestedFileNames().end()) {
        candidates = entry.value();
    } else {
        candidates = d->converter()->candidatesForCssValues(info,
                                                            xRes,
                                                            yRes);
        d->suggestedFileNames().insert(suggestedHash, candidates);
    }

    QVector<KoFFWWSConverter::FontFileEntry> fonts;
    lengths.clear();

    if (disableFontMatching && !candidates.isEmpty()) {
        fonts.append(candidates.first());
        lengths.append(text.size());
    } else {

        FcPatternSP p(FcPatternCreate());
        Q_FOREACH (const QString &family, info.families) {
            QByteArray utfData = family.toUtf8();
            const FcChar8 *vals = reinterpret_cast<FcChar8 *>(utfData.data());
            FcPatternAddString(p.data(), FC_FAMILY, vals);
        }

        {
            QByteArray fallbackBuf = QString("sans-serif").toUtf8();
            FcValue fallback;
            fallback.type = FcTypeString;
            fallback.u.s = reinterpret_cast<FcChar8 *>(fallbackBuf.data());
            FcPatternAddWeak(p.data(), FC_FAMILY, fallback, true);
        }

        for (int i = 0; i < candidates.size(); i++) {
            KoFFWWSConverter::FontFileEntry file = candidates.at(i);
            QByteArray utfData = file.fileName.toUtf8();
            const FcChar8 *vals = reinterpret_cast<FcChar8 *>(utfData.data());
            FcPatternAddString(p.data(), FC_FILE, vals);
            FcPatternAddInteger(p.data(), FC_INDEX, file.fontIndex);
        }

        if (info.slantMode == QFont::StyleItalic) {
            FcPatternAddInteger(p.data(), FC_SLANT, FC_SLANT_ITALIC);
        } else if (info.slantMode == QFont::StyleOblique) {
            FcPatternAddInteger(p.data(), FC_SLANT, FC_SLANT_OBLIQUE);
        } else {
            FcPatternAddInteger(p.data(), FC_SLANT, FC_SLANT_ROMAN);
        }
        FcPatternAddInteger(p.data(), FC_WEIGHT, FcWeightFromOpenType(info.weight));
        FcPatternAddInteger(p.data(), FC_WIDTH, info.width);

        double pixelSize = info.size*(qMin(xRes, yRes)/72.0);
        FcPatternAddDouble(p.data(), FC_PIXEL_SIZE, pixelSize);

        FcConfigSubstitute(nullptr, p.data(), FcMatchPattern);
        FcDefaultSubstitute(p.data());


        p = [&]() {
            const FcChar32 hash = FcPatternHash(p.data());
            QString patternHash = info.families.join("+")+QString::number(hash);
            // FCPatternHash breaks down when there's mutliple family names that start
            // with the same letter and are the same length (Butcherman and Babylonica, Eater and Elsie, all 4 on google fonts).
            const auto oldPattern = d->patterns().find(patternHash);
            if (oldPattern != d->patterns().end()) {
                return oldPattern.value();
            } else {
                d->patterns().insert(patternHash, p);
                return p;
            }
        }();

        FcResult result = FcResultNoMatch;
        FcCharSetSP charSet;
        FcFontSetSP fontSet = [&]() -> FcFontSetSP {
            const FcChar32 hash = FcPatternHash(p.data());
            QString patternHash = info.families.join("+")+QString::number(hash);
            const auto set = d->sets().find(patternHash);

            if (set != d->sets().end()) {
                return set.value();
            } else {
                FcCharSet *cs = nullptr;
                FcFontSetSP avalue(FcFontSort(FcConfigGetCurrent(), p.data(), FcFalse, &cs, &result));
                charSet.reset(cs);
                d->sets().insert(patternHash, avalue);
                return avalue;
            }
        }();

        if (text.isEmpty()) {
            for (int j = 0; j < fontSet->nfont; j++) {
                if (std::optional<KoFFWWSConverter::FontFileEntry> font = getFontFileEntry(fontSet->fonts[j])) {
                    fonts.append(std::move(*font));
                    lengths.append(0);
                    break;
                }
            }
        } else {
            FcCharSet *set = nullptr;
            QVector<int> familyValues(text.size());
            QVector<int> fallbackMatchValues(text.size());
            familyValues.fill(-1);
            fallbackMatchValues.fill(-1);

            // First, we're going to split up the text into graphemes. This is both
            // because the css spec requires it, but also because of why the css
            // spec requires it: graphemes' parts should not end up in seperate
            // runs, which they will if they get assigned different fonts,
            // potentially breaking ligatures and emoji sequences.
            QStringList graphemes = KoCssTextUtils::textToUnicodeGraphemeClusters(text, language);

            // Parse over the fonts and graphemes and try to see if we can get the
            // best match for a given grapheme.
            for (int i = 0; i < fontSet->nfont; i++) {

                double fontsize = 0.0;
                FcBool isScalable = false;
                FcPatternGetBool(fontSet->fonts[i], FC_SCALABLE, 0, &isScalable);
                FcPatternGetDouble(fontSet->fonts[i], FC_PIXEL_SIZE, 0, &fontsize);
                if (!isScalable && pixelSize != fontsize) {
                    // For some reason, FC will sometimes consider a smaller font pixel-size
                    // to be more relevant to the requested pattern than a bigger one. This
                    // skips those fonts, but it does mean that such pixel fonts would not
                    // be used for fallback.
                    continue;
                }
                if (FcPatternGetCharSet(fontSet->fonts[i], FC_CHARSET, 0, &set) == FcResultMatch) {
                    int index = 0;
                    Q_FOREACH (const QString &grapheme, graphemes) {

                        // Don't worry about matching controls directly,
                        // as they are not important to font-selection (and many
                        // fonts have no glyph entry for these)
                        if (const uint first = firstCharUcs4(grapheme); QChar::category(first) == QChar::Other_Control
                                || QChar::category(first) == QChar::Other_Format) {
                            index += grapheme.size();
                            continue;
                        }
                        int familyIndex = -1;
                        if (familyValues.at(index) == -1) {
                            int fallbackMatch = fallbackMatchValues.at(index);
                            Q_FOREACH (uint unicode, grapheme.toUcs4()) {
                                if (FcCharSetHasChar(set, unicode)) {
                                    familyIndex = i;
                                    if (fallbackMatch < 0) {
                                        fallbackMatch = i;
                                    }
                                } else {
                                    familyIndex = -1;
                                    break;
                                }
                            }
                            for (int k = 0; k < grapheme.size(); k++) {
                                familyValues[index + k] = familyIndex;
                                fallbackMatchValues[index + k] = fallbackMatch;
                            }
                        }
                        index += grapheme.size();
                    }
                    if (!familyValues.contains(-1)) {
                        break;
                    }
                }
            }

            // Remove the -1 entries.
            if (familyValues.contains(-1)) {
                int value = -1;
                Q_FOREACH (const int currentValue, familyValues) {
                    if (currentValue != value) {
                        value = currentValue;
                        break;
                    }
                }
                value = qMax(0, value);
                for (int i = 0; i < familyValues.size(); i++) {
                    if (familyValues.at(i) < 0) {
                        if (fallbackMatchValues.at(i) < 0) {
                            familyValues[i] = value;
                        } else {
                            familyValues[i] = fallbackMatchValues.at(i);
                        }
                    } else {
                        value = familyValues.at(i);
                    }
                }
            }

            // Get the filenames and lengths for the entries.
            int length = 0;
            int startIndex = 0;
            int lastIndex = familyValues.at(0);
            KoFFWWSConverter::FontFileEntry font{};
            if (std::optional<KoFFWWSConverter::FontFileEntry> f = getFontFileEntry(fontSet->fonts[lastIndex])) {
                font = std::move(*f);
            }
            for (int i = 0; i < familyValues.size(); i++) {
                if (lastIndex != familyValues.at(i)) {
                    lengths.append(text.mid(startIndex, length).size());
                    fonts.append(font);
                    startIndex = i;
                    length = 0;
                    lastIndex = familyValues.at(i);
                    if (std::optional<KoFFWWSConverter::FontFileEntry> f = getFontFileEntry(fontSet->fonts[lastIndex])) {
                        font = std::move(*f);
                    }
                }
                length += 1;
            }
            if (length > 0) {
                lengths.append(text.mid(startIndex, length).size());
                fonts.append(font);
            }
        }
    }

    std::vector<FT_FaceSP> faces;

    KIS_ASSERT_X(lengths.size() == fonts.size(),
                 "KoFontRegistry",
                 QString("Fonts and lengths don't have the same size. Fonts: %1. Length: %2")
                 .arg(fonts.size(), lengths.size()).toLatin1());

    for (int i = 0; i < lengths.size(); i++) {
        KoFFWWSConverter::FontFileEntry font = fonts.at(i);
        // For some reason, FontConfig will sometimes return the wrong file index.
        // We can in this case select the appropriate index, which should work as tcc collections share glyphs.
        for (int j = 0; j < candidates.size(); j++) {
            if (font.fileName == candidates.at(j).fileName) {
                font.fontIndex = candidates.at(j).fontIndex;
                break;
            }
        }

        const QString fontCacheEntry = font.fileName + "#" + QString::number(font.fontIndex) + "#" + modifications;
        auto entry = d->typeFaces().find(fontCacheEntry);
        if (entry != d->typeFaces().end()) {
            faces.emplace_back(entry.value());
        } else {
            FT_Face f = nullptr;
            QByteArray utfData = font.fileName.toUtf8();
            if (FT_New_Face(d->library().data(), utfData.data(), font.fontIndex, &f) == 0) {
                FT_FaceSP face(f);
                configureFaces({face}, info.size, info.fontSizeAdjust, xRes, yRes, info.computedAxisSettings());
                faces.emplace_back(face);
                d->typeFaces().insert(fontCacheEntry, face);
            }
        }
    }

    return faces;
}

bool KoFontRegistry::configureFaces(const std::vector<FT_FaceSP> &faces,
                                    qreal size,
                                    qreal fontSizeAdjust,
                                    quint32 xRes,
                                    quint32 yRes,
                                    const QMap<QString, qreal> &axisSettings)
{
    int errorCode = 0;
    const qreal ftFontUnit = 64.0;
    const qreal finalRes = qMin(xRes, yRes);
    const qreal scaleToPixel = finalRes / 72;
    Q_FOREACH (const FT_FaceSP &face, faces) {

        // set the charmap, by default Freetype will select a Unicode charmap, but when there's none, it doesn't set any and we need to select one manually.
        if (!face->charmap) {
            FT_CharMap map = nullptr;
            for (int i = 0; i < face->num_charmaps; i++) {
                FT_CharMap m = face->charmaps[i];
                if (!m) {
                    continue;
                }

                if (!map) {
                    map = m;
                    continue;
                }

                switch (m->encoding) {
                case FT_ENCODING_UNICODE:
                    map = m;
                    break;
                case FT_ENCODING_APPLE_ROMAN:
                case FT_ENCODING_ADOBE_LATIN_1:
                    if (!map || map->encoding != FT_ENCODING_UNICODE) {
                        map = m;
                    }
                    break;
                case FT_ENCODING_ADOBE_CUSTOM:
                case FT_ENCODING_MS_SYMBOL:
                    if (!map || (map->encoding != FT_ENCODING_UNICODE && map->encoding != FT_ENCODING_APPLE_ROMAN && map->encoding != FT_ENCODING_ADOBE_LATIN_1)) {
                        map = m;
                    }
                default:
                    break;
                }
            }
            FT_Set_Charmap(face.data(), map);
        }

        qreal adjustedSize = size;

        if (!FT_IS_SCALABLE(face)) {
            const qreal fontSizePixels = size * ftFontUnit * scaleToPixel;
            qreal sizeDelta = 0;
            int selectedIndex = -1;

            for (int i = 0; i < face->num_fixed_sizes; i++) {
                const qreal newDelta = qAbs((fontSizePixels)-face->available_sizes[i].x_ppem);
                if (newDelta < sizeDelta || i == 0) {
                    selectedIndex = i;
                    sizeDelta = newDelta;
                }
            }

            if (selectedIndex >= 0) {
                if (FT_HAS_COLOR(face)) {
                    const FT_Fixed scale = static_cast<FT_Fixed>(65535. * qreal(fontSizePixels)
                                                                 / qreal(face->available_sizes[selectedIndex].x_ppem));
                    FT_Matrix matrix;
                    matrix.xx = scale;
                    matrix.xy = 0;
                    matrix.yx = 0;
                    matrix.yy = scale;
                    FT_Vector v;
                    FT_Set_Transform(face.data(), &matrix, &v);
                }
                errorCode = FT_Select_Size(face.data(), selectedIndex);
            }
        } else {
            errorCode = FT_Set_Char_Size(face.data(), static_cast<FT_F26Dot6>(size * ftFontUnit), 0, xRes, yRes);
            hb_font_t_sp font(hb_ft_font_create_referenced(face.data()));
            hb_position_t xHeight = 0;
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_X_HEIGHT, &xHeight);
            if (xHeight > 0 && fontSizeAdjust > 0) {
                qreal aspect = xHeight / (size * ftFontUnit * scaleToPixel);
                adjustedSize = (fontSizeAdjust / aspect) * (size);
                errorCode = FT_Set_Char_Size(face.data(),
                                             static_cast<FT_F26Dot6>(adjustedSize * ftFontUnit),
                                             0,
                                             xRes,
                                             yRes);
            }
        }

        QMap<FT_Tag, qreal> tags;
        Q_FOREACH (const QString &tagName, axisSettings.keys()) {
            if (tagName.size() == 4) {
                const QByteArray utfData = tagName.toUtf8();
                const char *tag = utfData.data();
                if (tagName == "opsz" && !qFuzzyCompare(adjustedSize, size)) {
                    /**
                     *  There's something peculiar about this:
                     *  Var fonts can change their x-height depending
                     *  on the optical size selected. Meaning that the x-height adjusted size
                     *  could be wrong depending on the font and its x-height choices.
                     *  There's no reasonable way to fix this beyond cycling, and it is questionable
                     *  whether the font-size-adjust property is even meant to be that specific.
                     */
                    tags.insert(FT_MAKE_TAG(tag[0], tag[1], tag[2], tag[3]), adjustedSize);
                } else {
                    tags.insert(FT_MAKE_TAG(tag[0], tag[1], tag[2], tag[3]), axisSettings.value(tagName));
                }
            }
        }
        const FT_Tag ITALIC_TAG = FT_MAKE_TAG('i', 't', 'a', 'l');
        const FT_Tag SLANT_TAG = FT_MAKE_TAG('s', 'l', 'n', 't');
        const int axisMultiplier = 65535;
        if (FT_HAS_MULTIPLE_MASTERS(face)) {
            FT_MM_Var *amaster = nullptr;
            FT_Get_MM_Var(face.data(), &amaster);
            // note: this only works for opentype, as it uses
            // tag-based-selection.
            std::vector<FT_Fixed> designCoords(amaster->num_axis);
            for (FT_UInt i = 0; i < amaster->num_axis; i++) {
                FT_Var_Axis axis = amaster->axis[i];
                designCoords[i] = axis.def;
                if (tags.contains(axis.tag)) {
                    designCoords[i] = qBound(axis.minimum, long(tags.value(axis.tag) * axisMultiplier), axis.maximum);
                } else if (axis.tag == SLANT_TAG
                           && !tags.contains(SLANT_TAG)
                           && tags.value(ITALIC_TAG, 0) > 0) {
                    designCoords[i] = qBound(axis.minimum, long(-11.0 * axisMultiplier), axis.maximum);
                } else if (axis.tag == ITALIC_TAG
                           && !tags.contains(ITALIC_TAG)
                           && !qFuzzyCompare(tags.value(SLANT_TAG, 0), 0)) {
                    designCoords[i] = qBound(axis.minimum, long(1.0 * axisMultiplier), axis.maximum);
                }
            }
            FT_Set_Var_Design_Coordinates(face.data(), amaster->num_axis, designCoords.data());
            FT_Done_MM_Var(d->library().data(), amaster);
        }
    }
    return (errorCode == 0);
}

QList<KoFontFamilyWWSRepresentation> KoFontRegistry::collectRepresentations() const
{
    return d->converter()->collectFamilies();
}

std::optional<KoFontFamilyWWSRepresentation> KoFontRegistry::representationByFamilyName(const QString &familyName) const
{
    return d->converter()->representationByFamilyName(familyName);
}

std::optional<QString> KoFontRegistry::wwsNameByFamilyName(const QString familyName) const
{
    return d->converter()->wwsNameByFamilyName(familyName);
}

void KoFontRegistry::updateConfig()
{
    d->updateConfig();
}

QFont::Style KoFontRegistry::slantMode(FT_FaceSP face)
{
    constexpr unsigned OS2_ITALIC = 1u << 0; /// Is italic
    constexpr unsigned OS2_REGULAR = 1u << 6; /// Is truly regular (instead of italic or oblique)
    constexpr unsigned OS2_OBLIQUE = 1u << 9; // Is an oblique instead of an italic.

    if (FT_IS_SFNT(face.data())) {
        hb_font_t_sp hbFont(hb_ft_font_create_referenced(face.data()));
        bool isItalic = hb_style_get_value(hbFont.data(), HB_STYLE_TAG_ITALIC) > 0;
        bool isOblique = false;

        if (FT_HAS_MULTIPLE_MASTERS(face)) {
            isOblique = hb_style_get_value(hbFont.data(), HB_STYLE_TAG_SLANT_ANGLE) != 0;
        } else {
            TT_OS2 *os2Table = nullptr;
            os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
            if (os2Table) {
                isItalic = os2Table->fsSelection & OS2_ITALIC;
                isOblique = os2Table->fsSelection & OS2_OBLIQUE;
            }
            if (os2Table->fsSelection & OS2_REGULAR) {
                isItalic = false;
                isOblique = false;
            }
        }
        return isItalic? isOblique? QFont::StyleOblique: QFont::StyleItalic: QFont::StyleNormal;
    } else {
        return face->style_flags & FT_STYLE_FLAG_ITALIC? QFont::StyleItalic: QFont::StyleNormal;
    }
}

KoSvgText::FontMetrics KoFontRegistry::fontMetricsForCSSValues(KoCSSFontInfo info,
                                                               const bool isHorizontal, const KoSvgText::TextRendering rendering,
                                                               const QString &text,
                                                               quint32 xRes, quint32 yRes,
                                                               bool disableFontMatching, const QString &language)
{
    const QString suggestedHash = info.families.join(",")+":"+modificationsString(info, xRes, yRes)+language;
    KoSvgText::FontMetrics metrics;
    auto entry = d->fontMetrics().find(suggestedHash);
    if (entry != d->fontMetrics().end()) {
        metrics = entry.value();
    } else {
        QVector<int> lengths;
        const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
            lengths,
            info,
            text,
            xRes,
            yRes,
            disableFontMatching,
            language);

        if (faces.empty()) return KoSvgText::FontMetrics(info.size, isHorizontal);
        metrics = KoFontRegistry::generateFontMetrics(faces.front(), isHorizontal, KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale(language).script()), rendering);
        d->fontMetrics().insert(suggestedHash, metrics);
    }
    return metrics;
}

KoSvgText::FontMetrics KoFontRegistry::generateFontMetrics(FT_FaceSP face, bool isHorizontal, QString script, const KoSvgText::TextRendering rendering)
{
    KoSvgText::FontMetrics metrics;
    hb_direction_t dir = isHorizontal? HB_DIRECTION_LTR: HB_DIRECTION_TTB;
    QLatin1String scriptLatin1(script.toLatin1());
    hb_script_t scriptTag = hb_script_from_string(scriptLatin1.data(), scriptLatin1.size());
    hb_tag_t otScriptTag = HB_OT_TAG_DEFAULT_SCRIPT;
    hb_tag_t otLangTag = HB_OT_TAG_DEFAULT_LANGUAGE;
    uint maxCount = 1;
    hb_ot_tags_from_script_and_language(scriptTag, nullptr, &maxCount, &otScriptTag, &maxCount, &otLangTag);
    hb_font_t_sp font(hb_ft_font_create_referenced(face.data()));
    hb_face_t_sp hbFace(hb_ft_face_create_referenced(face.data()));

    const FT_Int32 faceLoadFlags = loadFlagsForFace(face.data(), isHorizontal, 0, rendering);

    metrics.isVertical = !isHorizontal;
    // Fontsize and advances.
    metrics.fontSize = isHorizontal? face.data()->size->metrics.y_ppem*64.0: face.data()->size->metrics.x_ppem*64.0;

    uint charIndex = FT_Get_Char_Index(face.data(), 0x20);
    if(charIndex > 0) {
        if (FT_Load_Glyph(face.data(), charIndex, faceLoadFlags) == FT_Err_Ok) {
            metrics.spaceAdvance = isHorizontal? face.data()->glyph->advance.x: face.data()->glyph->advance.y;
        } else {
            metrics.spaceAdvance = isHorizontal? metrics.fontSize/2: metrics.fontSize;
        }
    } else {
        metrics.spaceAdvance = isHorizontal? metrics.fontSize/2: metrics.fontSize;
    }

    charIndex = FT_Get_Char_Index(face.data(), '0');
    if(charIndex > 0) {
        if(FT_Load_Glyph(face.data(), charIndex, faceLoadFlags) == FT_Err_Ok) {
            metrics.zeroAdvance = isHorizontal? face.data()->glyph->advance.x: face.data()->glyph->advance.y;
        } else {
            metrics.zeroAdvance = isHorizontal? metrics.fontSize/2: metrics.fontSize;
        }
    } else {
        metrics.zeroAdvance = isHorizontal? metrics.fontSize/2: metrics.fontSize;
    }

    bool isIdeographic = false;
    charIndex = FT_Get_Char_Index(face.data(), 0x6C34);
    if(charIndex > 0) {
        if (FT_Load_Glyph(face.data(), charIndex, faceLoadFlags) == FT_Err_Ok) {
            metrics.ideographicAdvance = isHorizontal? face.data()->glyph->advance.x: face.data()->glyph->advance.y;
        } else {
            metrics.ideographicAdvance = metrics.fontSize;
        }
        isIdeographic = true;
    } else {
        metrics.ideographicAdvance = metrics.fontSize;
    }

    const bool useFallback = hb_version_atleast(4, 0, 0);

    // metrics

    QVector<hb_ot_metrics_tag_t> metricTags ({
                                              HB_OT_METRICS_TAG_X_HEIGHT,
                                              HB_OT_METRICS_TAG_CAP_HEIGHT,
                                              HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET,
                                              HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET,
                                              HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET,
                                              HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET,
                                              HB_OT_METRICS_TAG_UNDERLINE_OFFSET,
                                              HB_OT_METRICS_TAG_UNDERLINE_SIZE,
                                              HB_OT_METRICS_TAG_STRIKEOUT_OFFSET,
                                              HB_OT_METRICS_TAG_STRIKEOUT_SIZE,
                                          });

    if (isHorizontal) {
        metricTags.append(HB_OT_METRICS_TAG_HORIZONTAL_CARET_RISE);
        metricTags.append(HB_OT_METRICS_TAG_HORIZONTAL_CARET_RUN);
        metricTags.append(HB_OT_METRICS_TAG_HORIZONTAL_CARET_OFFSET);
        metrics.caretRise = 1;
        metrics.caretRun = 0;
    } else {
        metricTags.append(HB_OT_METRICS_TAG_VERTICAL_CARET_RISE);
        metricTags.append(HB_OT_METRICS_TAG_VERTICAL_CARET_RUN);
        metricTags.append(HB_OT_METRICS_TAG_VERTICAL_CARET_OFFSET);
        metrics.caretRise = 0;
        metrics.caretRun = 1;
    }

    for (auto it = metricTags.begin(); it!= metricTags.end(); it++) {
        char c[4];
        hb_tag_to_string(*it, c);
        const QLatin1String tagName(c, 4);
        hb_position_t origin = 0;
        if (useFallback) {
            hb_ot_metrics_get_position_with_fallback(font.data(), *it, &origin);
            metrics.setMetricsValueByTag(tagName, origin);
        } else if (hb_ot_metrics_get_position(font.data(), *it, &origin)) {
            metrics.setMetricsValueByTag(tagName, origin);
        }
    }

    // Baselines.

    const QVector<hb_ot_layout_baseline_tag_t> baselines ({
                HB_OT_LAYOUT_BASELINE_TAG_ROMAN,
                HB_OT_LAYOUT_BASELINE_TAG_HANGING,
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_FACE_BOTTOM_OR_LEFT,
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_FACE_TOP_OR_RIGHT,
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT,
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_TOP_OR_RIGHT,
                HB_OT_LAYOUT_BASELINE_TAG_MATH
    });

    QMap<QString, qint32> baselineVals;

    for (auto it = baselines.begin(); it!= baselines.end(); it++) {
        hb_position_t origin = 0;
        if (hb_ot_layout_get_baseline(font.data(), *it, dir, otScriptTag, otLangTag, &origin)) {
            std::vector<char> c(4);
            hb_tag_to_string(*it, c.data());
            baselineVals.insert(QString::fromLatin1(c.data(), 4), origin);
        }
    }

    //--- ascender/descender/linegap, and also ideographic em-box calculatation ---//

    hb_position_t ascender = 0;
    hb_position_t descender = 0;
    hb_position_t lineGap = 0;

    /**
     * There's 3 different definitions of the so-called vertical metrics, that is,
     * the ascender and descender for horizontally laid out script. WinAsc & Desc,
     * HHAE asc&desc, and OS/2... we need the last one, but harfbuzz doesn't return
     * it unless there's a flag set in the font, which is missing in a lot of fonts
     * that were from the transitional period, like Deja Vu Sans. Hence we need to get
     * the OS/2 table and calculate the values manually (and fall back in various ways).
     *
     * https://www.w3.org/TR/css-inline-3/#ascent-descent
     * https://www.w3.org/TR/CSS2/visudet.html#sTypoAscender
     * https://wiki.inkscape.org/wiki/Text_Rendering_Notes#Ascent_and_Descent
     *
     * Related HB issue: https://github.com/harfbuzz/harfbuzz/issues/1920
     */
    TT_OS2 *os2Table = nullptr;
    os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
    if (os2Table) {
        int yscale = face.data()->size->metrics.y_scale;

        ascender = FT_MulFix(os2Table->sTypoAscender, yscale);
        descender = FT_MulFix(os2Table->sTypoDescender, yscale);
        lineGap = FT_MulFix(os2Table->sTypoLineGap, yscale);
    }

    constexpr unsigned USE_TYPO_METRICS = 1u << 7;
    if (!os2Table || os2Table->version == 0xFFFFU || !(os2Table->fsSelection & USE_TYPO_METRICS)) {
        hb_position_t altAscender = 0;
        hb_position_t altDescender = 0;
        hb_position_t altLineGap = 0;
        if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER, &altAscender)) {
            altAscender = face.data()->ascender;
        }
        if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER, &altDescender)) {
            altDescender = face.data()->descender;
        }
        if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_LINE_GAP, &altLineGap)) {
            altLineGap = face.data()->height - (altAscender-altDescender);
        }

        // Some fonts have sTypo metrics that are too small compared
        // to the HHEA values which make the default line height too
        // tight (e.g. Microsoft JhengHei, Source Han Sans), so we
        // compare them and take the ones that are larger.
        if (!os2Table || (altAscender - altDescender + altLineGap) > (ascender - descender + lineGap)) {
            ascender = altAscender;
            descender = altDescender;
            lineGap = altLineGap;
        }
    }

    // Because the ideographic em-box is so important to SVG 2 vertical line calculation,
    // I am manually calculating it here, following
    // https://learn.microsoft.com/en-us/typography/opentype/spec/baselinetags#ideographic-em-box
    const QString ideoBottom("ideo");
    const QString ideoTop("idtp");
    const QString alphabetic("romn");
    const QString ideoCenter("Idce");
    const QString hang("hang");
    const QString math("math");

    if (baselineVals.keys().contains(ideoBottom) && !baselineVals.keys().contains(ideoTop)) {
        baselineVals.insert(ideoTop, baselineVals.value(ideoBottom)+metrics.fontSize);
    } else if (!baselineVals.keys().contains(ideoBottom) && baselineVals.keys().contains(ideoTop)) {
        baselineVals.insert(ideoBottom, baselineVals.value(ideoTop)-metrics.fontSize);
    } else if (!baselineVals.keys().contains(ideoBottom) && !baselineVals.keys().contains(ideoTop)){

        if (!isIdeographic && isHorizontal) {
            hb_blob_t_sp dLang(hb_ot_meta_reference_entry( hbFace.data() , HB_OT_META_TAG_DESIGN_LANGUAGES));
            uint length = hb_blob_get_length(dLang.data());
            QByteArray ba(hb_blob_get_data(dLang.data(), &length), length);

            const QString designLang = QString::fromLatin1(ba).trimmed();

            if (!designLang.isEmpty()) {
                // This assumes a font where there's design language metadata, but no water glyph.
                // In theory could happen with the non-han cjk fonts.
                const QStringList cjkScripts {
                    KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::HanScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::JapaneseScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::TraditionalHanScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::SimplifiedHanScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::HanWithBopomofoScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::BopomofoScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::HangulScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::KatakanaScript),
                            KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::HiraganaScript),
                };
                Q_FOREACH (const QString cjk, cjkScripts) {
                    if (cjk.isEmpty()) continue;
                    if (designLang.contains(cjk.trimmed())) {
                        isIdeographic = true;
                        break;
                    }
                }
            }
        }

        if (isIdeographic && isHorizontal) {
            baselineVals.insert(ideoTop, ascender);
            baselineVals.insert(ideoBottom, descender);
            if (!baselineVals.keys().contains(alphabetic)) {
                baselineVals.insert(alphabetic, 0);
            }
            if (!baselineVals.keys().contains(hang)) {
                baselineVals.insert(hang, baselineVals.value(alphabetic)+metrics.fontSize*0.6);
            }
        } else if (isHorizontal) {
            const qreal alphabeticMultiplier = qreal(ascender+descender)/metrics.fontSize;
            qint32 top = qMax(baselineVals.value(hang, metrics.capHeight), qint32(qRound(ascender*alphabeticMultiplier)));
            baselineVals.insert(ideoTop, top);
            baselineVals.insert(ideoBottom, top-metrics.fontSize);
        } else {
            baselineVals.insert(ideoTop, metrics.fontSize);
            baselineVals.insert(ideoBottom, 0);
            if (!baselineVals.keys().contains(alphabetic)) {
                const qreal alphabeticMultiplier = qreal(ascender+descender)/metrics.fontSize;
                baselineVals.insert(alphabetic, (-descender)*alphabeticMultiplier);
            }
            if (!baselineVals.keys().contains(hang)) {
                baselineVals.insert(hang, baselineVals.value(alphabetic)+metrics.fontSize*0.6);
            }
        }
    }
    baselineVals.insert(ideoCenter, (baselineVals.value(ideoTop) + baselineVals.value(ideoBottom))/2);
    if (!isHorizontal && !baselineVals.keys().contains(math)) {
        baselineVals.insert(math, baselineVals.value(ideoCenter));
    }

    if (useFallback) {
        for (auto it = baselines.begin(); it!= baselines.end(); it++) {
            char c[4];
            hb_tag_to_string(*it, c);
            const QString tagName = QString::fromLatin1(c, 4);
            if (!baselineVals.keys().contains(tagName)) {
                hb_position_t origin = 0;
                hb_ot_layout_get_baseline_with_fallback(font.data(), *it, dir, otScriptTag, otLangTag, &origin);
                baselineVals.insert(tagName, origin);
            }
        }
    }

    Q_FOREACH(const QString key, baselineVals.keys()) {
        metrics.setBaselineValueByTag(key, baselineVals.value(key));
    }

    if (!isHorizontal) {
        // SVG 2.0 explicitely requires vertical ascent/descent to be tied to the ideographic em box.
        ascender = metrics.ideographicOverBaseline;
        descender = metrics.ideographicUnderBaseline;

        // Default microsoft CJK fonts have the vertical ascent and descent be the same as the horizontal
        // ascent and descent, so we 'normalize' the ascender and descender to be half the total height.
        qreal height = ascender - descender;
        ascender = height*0.5;
        descender = -ascender;
    }
    if (ascender == 0 && descender == 0) {
        ascender = face->size->metrics.ascender;
        descender = face->size->metrics.descender;
        qreal height = ascender - descender;
        lineGap = face->size->metrics.height - height;
        if (!isHorizontal) {
            ascender = height * 0.5;
            descender = -ascender;
        }
    }
    metrics.ascender = ascender;
    metrics.descender = descender;
    metrics.lineGap = lineGap;

    if (isHorizontal) {
        metrics.offsetMetricsToNewOrigin(KoSvgText::BaselineAlphabetic);
    } else {
        // because we normalize, we need to add here.
        metrics.ascender += metrics.ideographicCenterBaseline;
        metrics.descender += metrics.ideographicCenterBaseline;
        metrics.offsetMetricsToNewOrigin(KoSvgText::BaselineCentral);
    }

    return metrics;
}

int32_t KoFontRegistry::loadFlagsForFace(FT_Face face, bool isHorizontal, int32_t loadFlags, const KoSvgText::TextRendering rendering)
{
    FT_Int32 faceLoadFlags = loadFlags;

    if (rendering == KoSvgText::RenderingGeometricPrecision || rendering == KoSvgText::RenderingAuto) {
        // without load_no_hinting, the advance and offset will be rounded
        // to nearest pixel, which we don't want as we're using the vector
        // outline.
        faceLoadFlags |= FT_LOAD_NO_HINTING;

        // Disable embedded bitmaps because they _do not_ follow geometric
        // precision, but is focused on legibility.
        // This does not affect bitmap-only fonts.
        faceLoadFlags |= FT_LOAD_NO_BITMAP;
    } else if (rendering == KoSvgText::RenderingOptimizeSpeed) {
        faceLoadFlags |= FT_LOAD_TARGET_MONO;
    } else {
        // When using hinting, sometimes the bounding box does not encompass the
        // drawn glyphs properly.
        // The default hinting works best for vertical, while the 'light'
        // hinting mode works best for horizontal.
        if (isHorizontal) {
            faceLoadFlags |= FT_LOAD_TARGET_LIGHT;
        }
    }
    if (FT_HAS_COLOR(face)) {
        faceLoadFlags |= FT_LOAD_COLOR;
    }
    if (!isHorizontal && FT_HAS_VERTICAL(face)) {
        faceLoadFlags |= FT_LOAD_VERTICAL_LAYOUT;
    }
    if (!FT_IS_SCALABLE(face)) {
        // This is needed for the CBDT version of Noto Color Emoji
        faceLoadFlags &= ~FT_LOAD_NO_BITMAP;
    }
    return faceLoadFlags;
}

void KoFontRegistry::getCssDataForPostScriptName(const QString postScriptName, QString *foundPostScriptName, QString *cssFontFamily, int &cssFontWeight, int &cssFontWidth, bool &cssItalic)
{
    FcPatternUP p(FcPatternCreate());
    QByteArray utfData = postScriptName.toUtf8();
    const FcChar8 *vals = reinterpret_cast<FcChar8 *>(utfData.data());
    FcPatternAddString(p.data(), FC_POSTSCRIPT_NAME, vals);
    FcDefaultSubstitute(p.data());

    FcResult result = FcResultNoMatch;
    FcPatternUP match(FcFontMatch(FcConfigGetCurrent(), p.data(), &result));
    if (result != FcResultNoMatch) {
        FcChar8 *fileValue = nullptr;
        if (FcPatternGetString(match.data(), FC_FAMILY, 0, &fileValue) == FcResultMatch) {
            *cssFontFamily = QString(reinterpret_cast<char *>(fileValue));
        } else {
            *cssFontFamily = postScriptName;
        }
        if (FcPatternGetString(match.data(), FC_POSTSCRIPT_NAME, 0, &fileValue) == FcResultMatch) {
            *foundPostScriptName = QString(reinterpret_cast<char *>(fileValue));
        } else {
            *foundPostScriptName = postScriptName;
        }
        int value;
        if (FcPatternGetInteger(match.data(), FC_WEIGHT, 0, &value) == FcResultMatch) {
            cssFontWeight = FcWeightToOpenType(value);
        }
        if (FcPatternGetInteger(match.data(), FC_WIDTH, 0, &value) == FcResultMatch) {
            cssFontWidth = value;
        }
        if (FcPatternGetInteger(match.data(), FC_SLANT, 0, &value) == FcResultMatch) {
            cssItalic = value != FC_SLANT_ROMAN;
        }
    } else {
        *cssFontFamily = postScriptName;
    }
}

bool KoFontRegistry::addFontFilePathToRegistery(const QString &path)
{
    const QByteArray utfData = path.toUtf8();
    const FcChar8 *vals = reinterpret_cast<const FcChar8 *>(utfData.data());
    bool success = false;
    if (FcConfigAppFontAddFile(d->config().data(), vals)) {
        success = d->reloadConverter();
    }
    return success;
}

bool KoFontRegistry::addFontFileDirectoryToRegistery(const QString &path)
{
    const QByteArray utfData = path.toUtf8();
    const FcChar8 *vals = reinterpret_cast<const FcChar8 *>(utfData.data());
    bool success = false;
    if (FcConfigAppFontAddDir(d->config().data(), vals)) {
        success = d->reloadConverter();
    }
    return success;
}
