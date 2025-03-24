/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoWritingSystemUtils.h"
#include <QRegExp>

static QMap<QFontDatabase::WritingSystem, QString> WRITINGSYSTEM_SCRIPT_MAP {
    {{QFontDatabase::Any},{"Zyyy"}},
    {{QFontDatabase::Latin},{"Latn"}},
    {{QFontDatabase::Greek},{"Grek"}},
    {{QFontDatabase::Cyrillic},{"Cyrl"}},
    {{QFontDatabase::Armenian},{"Armn"}},
    {{QFontDatabase::Hebrew},{"Hebr"}},
    {{QFontDatabase::Arabic},{"Arab"}},
    {{QFontDatabase::Syriac},{"Syrc"}},
    {{QFontDatabase::Thaana},{"Thaa"}},
    {{QFontDatabase::Devanagari},{"Deva"}},
    {{QFontDatabase::Bengali},{"Beng"}},
    {{QFontDatabase::Gurmukhi},{"Guru"}},
    {{QFontDatabase::Gujarati},{"Gujr"}},
    {{QFontDatabase::Oriya},{"Orya"}},
    {{QFontDatabase::Tamil},{"Taml"}},
    {{QFontDatabase::Telugu},{"Telu"}},
    {{QFontDatabase::Kannada},{"Knda"}},
    {{QFontDatabase::Malayalam},{"Mylm"}},
    {{QFontDatabase::Sinhala},{"Sinh"}},
    {{QFontDatabase::Thai},{"Thai"}},
    {{QFontDatabase::Lao},{"Laoo"}},
    {{QFontDatabase::Tibetan},{"Tibt"}},
    {{QFontDatabase::Myanmar},{"Mymr"}},
    {{QFontDatabase::Georgian},{"Geor"}},
    {{QFontDatabase::Khmer},{"Khmr"}},
    {{QFontDatabase::SimplifiedChinese},{"Hans"}},
    {{QFontDatabase::TraditionalChinese},{"Hant"}},
    {{QFontDatabase::Japanese},{"Jpan"}},
    {{QFontDatabase::Korean},{"Kore"}},
    {{QFontDatabase::Ogham},{"Ogam"}},
    {{QFontDatabase::Runic},{"Runr"}},
    {{QFontDatabase::Nko},{"Nkoo"}},
    /*{{QFontDatabase::Symbol},{"Zsye"}}, Symbol refers to the wingdings fonts, not actually unicode scripts.*/
    {{QFontDatabase::Vietnamese},{"Latn"}},
};

static QMap<QLocale::Script, QString> QLOCALE_SCRIPT_MAP {
    {{QLocale::LatinScript},{"Latn"}},
    {{QLocale::GreekScript},{"Grek"}},
    {{QLocale::CyrillicScript},{"Cyrl"}},
    {{QLocale::ArmenianScript},{"Armn"}},
    {{QLocale::HebrewScript},{"Hebr"}},
    {{QLocale::ArabicScript},{"Arab"}},
    {{QLocale::SyriacScript},{"Syrc"}},
    {{QLocale::ThaanaScript},{"Thaa"}},
    {{QLocale::DevanagariScript},{"Deva"}},
    {{QLocale::BengaliScript},{"Beng"}},
    {{QLocale::GurmukhiScript},{"Guru"}},
    {{QLocale::GujaratiScript},{"Gujr"}},
    {{QLocale::OriyaScript},{"Orya"}},
    {{QLocale::TamilScript},{"Taml"}},
    {{QLocale::TeluguScript},{"Telu"}},
    {{QLocale::KannadaScript},{"Knda"}},
    {{QLocale::MalayalamScript},{"Mylm"}},
    {{QLocale::SinhalaScript},{"Sinh"}},
    {{QLocale::ThaiScript},{"Thai"}},
    {{QLocale::LaoScript},{"Laoo"}},
    {{QLocale::TibetanScript},{"Tibt"}},
    {{QLocale::MyanmarScript},{"Mymr"}},
    {{QLocale::GeorgianScript},{"Geor"}},
    {{QLocale::KhmerScript},{"Khmr"}},
    {{QLocale::SimplifiedChineseScript},{"Hans"}},
    {{QLocale::TraditionalChineseScript},{"Hant"}},
    {{QLocale::JapaneseScript},{"Jpan"}},
    {{QLocale::KoreanScript},{"Kore"}},
    {{QLocale::OghamScript},{"Ogam"}},
    {{QLocale::RunicScript},{"Runr"}},
    {{QLocale::NkoScript},{"Nkoo"}},

    {{QLocale::DeseretScript},{"Dsrt"}},
    {{QLocale::MongolianScript},{"Mong"}},
    {{QLocale::TifinaghScript},{"Tfng"}},
    {{QLocale::CherokeeScript},{"Cher"}},
    {{QLocale::EthiopicScript},{"Ethi"}},
    {{QLocale::YiScript},{"Yiii"}},
    {{QLocale::VaiScript},{"Vaii"}},
    {{QLocale::AvestanScript},{"Avst"}},
    {{QLocale::BalineseScript},{"Bali"}},
    {{QLocale::BamumScript},{"Bamu"}},
    {{QLocale::BopomofoScript},{"Bopo"}},
    {{QLocale::BrahmiScript},{"Brah"}},
    {{QLocale::BugineseScript},{"Bugi"}},
    {{QLocale::BuhidScript},{"Buhd"}},
    {{QLocale::CanadianAboriginalScript},{"Cans"}},
    {{QLocale::CarianScript},{"Cari"}},
    {{QLocale::ChakmaScript},{"Cakm"}},
    {{QLocale::ChamScript},{"Cham"}},
    {{QLocale::CopticScript},{"Copt"}},
    {{QLocale::CypriotScript},{"Cprt"}},
    {{QLocale::EgyptianHieroglyphsScript},{"Egyp"}},
    {{QLocale::FraserScript},{"Lisu"}},
    {{QLocale::GlagoliticScript},{"Glag"}},
    {{QLocale::GothicScript},{"Goth"}},
    {{QLocale::HanScript},{"Hani"}},
    {{QLocale::HangulScript},{"Hang"}},
    {{QLocale::HanunooScript},{"Hano"}},
    {{QLocale::ImperialAramaicScript},{"Armi"}},
    {{QLocale::InscriptionalPahlaviScript},{"Phli"}},
    {{QLocale::InscriptionalParthianScript},{"Prti"}},
    {{QLocale::JavaneseScript},{"Java"}},
    {{QLocale::KaithiScript},{"Kthi"}},
    {{QLocale::KatakanaScript},{"Kana"}},
    {{QLocale::KayahLiScript},{"Kali"}},
    {{QLocale::KharoshthiScript},{"Khar"}},
    {{QLocale::LannaScript},{"Lana"}},
    {{QLocale::LepchaScript},{"Lepc"}},
    {{QLocale::LimbuScript},{"Limb"}},
    {{QLocale::LinearBScript},{"Linb"}},
    {{QLocale::LycianScript},{"Lyci"}},
    {{QLocale::LydianScript},{"Lydi"}},
    {{QLocale::MandaeanScript},{"Mand"}},
    {{QLocale::MeiteiMayekScript},{"Mtei"}},
    {{QLocale::MeroiticScript},{"Mero"}},
    {{QLocale::MeroiticCursiveScript},{"Merc"}},
    {{QLocale::NewTaiLueScript},{"Talu"}},
    {{QLocale::OlChikiScript},{"Olck"}},
    {{QLocale::OldItalicScript},{"Ital"}},
    {{QLocale::OldPersianScript},{"Xpeo"}},
    {{QLocale::OldSouthArabianScript},{"Sarb"}},
    {{QLocale::OrkhonScript},{"Orkh"}},
    {{QLocale::OsmanyaScript},{"Osma"}},
    {{QLocale::PhagsPaScript},{"Phag"}},
    {{QLocale::PhoenicianScript},{"Phnx"}},
    {{QLocale::PollardPhoneticScript},{"Plrd"}},
    {{QLocale::RejangScript},{"Rjng"}},
    {{QLocale::SamaritanScript},{"Samr"}},
    {{QLocale::SaurashtraScript},{"Saur"}},
    {{QLocale::SharadaScript},{"Shrd"}},
    {{QLocale::ShavianScript},{"Shaw"}},
    {{QLocale::SoraSompengScript},{"Sora"}},
    {{QLocale::CuneiformScript},{"Xsux"}},
    {{QLocale::SundaneseScript},{"Sund"}},
    {{QLocale::SylotiNagriScript},{"Sylo"}},
    {{QLocale::TagalogScript},{"Tglg"}},
    {{QLocale::TagbanwaScript},{"Tagb"}},
    {{QLocale::TaiLeScript},{"Tale"}},
    {{QLocale::TaiVietScript},{"Tavt"}},
    {{QLocale::TakriScript},{"Takr"}},
    {{QLocale::UgariticScript},{"Ugar"}},
    {{QLocale::BrailleScript},{"Brai"}},
    {{QLocale::HiraganaScript},{"Hira"}},
    {{QLocale::CaucasianAlbanianScript},{"Aghb"}},
    {{QLocale::BassaVahScript},{"Bass"}},
    {{QLocale::DuployanScript},{"Dupl"}},
    {{QLocale::ElbasanScript},{"Elba"}},
    {{QLocale::GranthaScript},{"Gran"}},
    {{QLocale::PahawhHmongScript},{"Hmng"}},
    {{QLocale::KhojkiScript},{"Khoi"}},
    {{QLocale::LinearAScript},{"Lina"}},
    {{QLocale::MahajaniScript},{"Mahj"}},
    {{QLocale::ManichaeanScript},{"Mani"}},
    {{QLocale::MendeKikakuiScript},{"Mend"}},
    {{QLocale::ModiScript},{"Modi"}},
    {{QLocale::MroScript},{"Mroo"}},
    {{QLocale::OldNorthArabianScript},{"Narb"}},
    {{QLocale::NabataeanScript},{"Nbat"}},
    {{QLocale::PalmyreneScript},{"Palm"}},
    {{QLocale::PauCinHauScript},{"Pauc"}},
    {{QLocale::PsalterPahlaviScript},{"Phlp"}},
    {{QLocale::KhudawadiScript},{"Sind"}},
    {{QLocale::TirhutaScript},{"Tirh"}},
    {{QLocale::VarangKshitiScript},{"Wara"}},
    {{QLocale::AhomScript},{"Ahom"}},
    {{QLocale::AnatolianHieroglyphsScript},{"Hluw"}},
    {{QLocale::HatranScript},{"Hatr"}},
    {{QLocale::MultaniScript},{"Mult"}},
    {{QLocale::OldHungarianScript},{"Hung"}},
    {{QLocale::SignWritingScript},{"Sgnw"}},
    {{QLocale::AdlamScript},{"Adlm"}},
    {{QLocale::BhaiksukiScript},{"Bhks"}},
    {{QLocale::BatakScript},{"Batk"}},
    {{QLocale::MarchenScript},{"Marc"}},
    {{QLocale::NewaScript},{"Newa"}},
    {{QLocale::OsageScript},{"Osge"}},
    {{QLocale::TangutScript},{"Tang"}},
    {{QLocale::HanWithBopomofoScript},{"Hanb"}},
    {{QLocale::JamoScript},{"Jamo"}},
};

static QMap<QChar::Script, QString> QCHAR_SCRIPT_MAP {
    {{QChar::Script_Latin},{"Latn"}},
    {{QChar::Script_Greek},{"Grek"}},
    {{QChar::Script_Cyrillic},{"Cyrl"}},
    {{QChar::Script_Armenian},{"Armn"}},
    {{QChar::Script_Hebrew},{"Hebr"}},
    {{QChar::Script_Arabic},{"Arab"}},
    {{QChar::Script_Syriac},{"Syrc"}},
    {{QChar::Script_Thaana},{"Thaa"}},
    {{QChar::Script_Devanagari},{"Deva"}},
    {{QChar::Script_Bengali},{"Beng"}},
    {{QChar::Script_Gurmukhi},{"Guru"}},
    {{QChar::Script_Gujarati},{"Gujr"}},
    {{QChar::Script_Oriya},{"Orya"}},
    {{QChar::Script_Tamil},{"Taml"}},
    {{QChar::Script_Telugu},{"Telu"}},
    {{QChar::Script_Kannada},{"Knda"}},
    {{QChar::Script_Malayalam},{"Mylm"}},
    {{QChar::Script_Sinhala},{"Sinh"}},
    {{QChar::Script_Thai},{"Thai"}},
    {{QChar::Script_Lao},{"Laoo"}},
    {{QChar::Script_Tibetan},{"Tibt"}},
    {{QChar::Script_Myanmar},{"Mymr"}},
    {{QChar::Script_Georgian},{"Geor"}},
    {{QChar::Script_Khmer},{"Khmr"}},
    {{QChar::Script_Ogham},{"Ogam"}},
    {{QChar::Script_Runic},{"Runr"}},
    {{QChar::Script_Nko},{"Nkoo"}},

    {{QChar::Script_Deseret},{"Dsrt"}},
    {{QChar::Script_Mongolian},{"Mong"}},
    {{QChar::Script_Tifinagh},{"Tfng"}},
    {{QChar::Script_Cherokee},{"Cher"}},
    {{QChar::Script_Ethiopic},{"Ethi"}},
    {{QChar::Script_Yi},{"Yiii"}},
    {{QChar::Script_Vai},{"Vaii"}},
    {{QChar::Script_Avestan},{"Avst"}},
    {{QChar::Script_Balinese},{"Bali"}},
    {{QChar::Script_Bamum},{"Bamu"}},
    {{QChar::Script_Bopomofo},{"Bopo"}},
    {{QChar::Script_Brahmi},{"Brah"}},
    {{QChar::Script_Buginese},{"Bugi"}},
    {{QChar::Script_Buhid},{"Buhd"}},
    {{QChar::Script_CanadianAboriginal},{"Cans"}},
    {{QChar::Script_Carian},{"Cari"}},
    {{QChar::Script_Chakma},{"Cakm"}},
    {{QChar::Script_Cham},{"Cham"}},
    {{QChar::Script_Coptic},{"Copt"}},
    {{QChar::Script_Cypriot},{"Cprt"}},
    {{QChar::Script_EgyptianHieroglyphs},{"Egyp"}},
    {{QChar::Script_Lisu},{"Lisu"}},
    {{QChar::Script_Glagolitic},{"Glag"}},
    {{QChar::Script_Gothic},{"Goth"}},
    {{QChar::Script_Han},{"Hani"}},
    {{QChar::Script_Hangul},{"Hang"}},
    {{QChar::Script_Hanunoo},{"Hano"}},
    {{QChar::Script_ImperialAramaic},{"Armi"}},
    {{QChar::Script_InscriptionalPahlavi},{"Phli"}},
    {{QChar::Script_InscriptionalParthian},{"Prti"}},
    {{QChar::Script_Javanese},{"Java"}},
    {{QChar::Script_Kaithi},{"Kthi"}},
    {{QChar::Script_Katakana},{"Kana"}},
    {{QChar::Script_KayahLi},{"Kali"}},
    {{QChar::Script_Kharoshthi},{"Khar"}},
    {{QChar::Script_TaiTham}, {"Lana"}},
    {{QChar::Script_Lepcha},{"Lepc"}},
    {{QChar::Script_Limbu},{"Limb"}},
    {{QChar::Script_LinearB},{"Linb"}},
    {{QChar::Script_Lycian},{"Lyci"}},
    {{QChar::Script_Lydian},{"Lydi"}},
    {{QChar::Script_Mandaic},{"Mand"}},
    {{QChar::Script_MeeteiMayek},{"Mtei"}},
    {{QChar::Script_MeroiticHieroglyphs},{"Mero"}},
    {{QChar::Script_MeroiticCursive},{"Merc"}},
    {{QChar::Script_NewTaiLue},{"Talu"}},
    {{QChar::Script_OlChiki},{"Olck"}},
    {{QChar::Script_OldItalic},{"Ital"}},
    {{QChar::Script_OldPersian},{"Xpeo"}},
    {{QChar::Script_OldSouthArabian},{"Sarb"}},
    {{QChar::Script_OldTurkic},{"Orkh"}},
    {{QChar::Script_Osmanya},{"Osma"}},
    {{QChar::Script_PhagsPa},{"Phag"}},
    {{QChar::Script_Phoenician},{"Phnx"}},
    {{QChar::Script_Miao},{"Plrd"}},
    {{QChar::Script_Rejang},{"Rjng"}},
    {{QChar::Script_Samaritan},{"Samr"}},
    {{QChar::Script_Saurashtra},{"Saur"}},
    {{QChar::Script_Sharada},{"Shrd"}},
    {{QChar::Script_Shavian},{"Shaw"}},
    {{QChar::Script_SoraSompeng},{"Sora"}},
    {{QChar::Script_Cuneiform},{"Xsux"}},
    {{QChar::Script_Sundanese},{"Sund"}},
    {{QChar::Script_SylotiNagri},{"Sylo"}},
    {{QChar::Script_Tagalog},{"Tglg"}},
    {{QChar::Script_Tagbanwa},{"Tagb"}},
    {{QChar::Script_TaiLe},{"Tale"}},
    {{QChar::Script_TaiViet},{"Tavt"}},
    {{QChar::Script_Takri},{"Takr"}},
    {{QChar::Script_Ugaritic},{"Ugar"}},
    {{QChar::Script_Braille},{"Brai"}},
    {{QChar::Script_Hiragana},{"Hira"}},
    {{QChar::Script_CaucasianAlbanian},{"Aghb"}},
    {{QChar::Script_BassaVah},{"Bass"}},
    {{QChar::Script_Duployan},{"Dupl"}},
    {{QChar::Script_Elbasan},{"Elba"}},
    {{QChar::Script_Grantha},{"Gran"}},
    {{QChar::Script_PahawhHmong},{"Hmng"}},
    {{QChar::Script_Khojki},{"Khoi"}},
    {{QChar::Script_LinearA},{"Lina"}},
    {{QChar::Script_Mahajani},{"Mahj"}},
    {{QChar::Script_Manichaean},{"Mani"}},
    {{QChar::Script_MendeKikakui},{"Mend"}},
    {{QChar::Script_Modi},{"Modi"}},
    {{QChar::Script_Mro},{"Mroo"}},
    {{QChar::Script_OldNorthArabian},{"Narb"}},
    {{QChar::Script_Nabataean},{"Nbat"}},
    {{QChar::Script_Palmyrene},{"Palm"}},
    {{QChar::Script_PauCinHau},{"Pauc"}},
    {{QChar::Script_PsalterPahlavi},{"Phlp"}},
    {{QChar::Script_Khudawadi},{"Sind"}},
    {{QChar::Script_Tirhuta},{"Tirh"}},
    {{QChar::Script_WarangCiti},{"Wara"}},
    {{QChar::Script_Ahom},{"Ahom"}},
    {{QChar::Script_AnatolianHieroglyphs},{"Hluw"}},
    {{QChar::Script_Hatran},{"Hatr"}},
    {{QChar::Script_Multani},{"Mult"}},
    {{QChar::Script_OldHungarian},{"Hung"}},
    {{QChar::Script_SignWriting},{"Sgnw"}},
    {{QChar::Script_Adlam},{"Adlm"}},
    {{QChar::Script_Bhaiksuki},{"Bhks"}},
    {{QChar::Script_Batak},{"Batk"}},
    {{QChar::Script_Marchen},{"Marc"}},
    {{QChar::Script_Newa},{"Newa"}},
    {{QChar::Script_Osage},{"Osge"}},
    {{QChar::Script_Tangut},{"Tang"}},

    {{QChar::Script_MasaramGondi},{"Gonm"}},
    {{QChar::Script_Nushu},{"Nshu"}},
    {{QChar::Script_Soyombo},{"Soyo"}},
    {{QChar::Script_ZanabazarSquare},{"Zanb"}},

    {{QChar::Script_Dogra},{"Dogr"}},
    {{QChar::Script_GunjalaGondi},{"Gong"}},
    {{QChar::Script_HanifiRohingya},{"Rogh"}},
    {{QChar::Script_Makasar},{"Maka"}},
    {{QChar::Script_Medefaidrin},{"Medf"}},
    {{QChar::Script_OldSogdian},{"Sogo"}},
    {{QChar::Script_Sogdian},{"Sogd"}},
    {{QChar::Script_Elymaic},{"Elym"}},
    {{QChar::Script_Nandinagari},{"Nand"}},
    {{QChar::Script_NyiakengPuachueHmong},{"Hmnp"}},
    {{QChar::Script_Wancho},{"Wcho"}},

    {{QChar::Script_Chorasmian},{"Chrs"}},
    {{QChar::Script_DivesAkuru},{"Diak"}},
    {{QChar::Script_KhitanSmallScript},{"Kits"}},
    {{QChar::Script_Yezidi},{"Yezi"}},
};

QString KoWritingSystemUtils::scriptTagForWritingSystem(QFontDatabase::WritingSystem system) {
    return WRITINGSYSTEM_SCRIPT_MAP.value(system);
}

QFontDatabase::WritingSystem KoWritingSystemUtils::writingSystemForScriptTag(const QString &tag)
{
    return WRITINGSYSTEM_SCRIPT_MAP.key(tag, QFontDatabase::Any);
}

QString KoWritingSystemUtils::scriptTagForQLocaleScript(QLocale::Script script)
{
    return QLOCALE_SCRIPT_MAP.value(script);
}

QLocale::Script KoWritingSystemUtils::scriptForScriptTag(const QString &tag)
{
    return QLOCALE_SCRIPT_MAP.key(tag, QLocale::AnyScript);
}

QString KoWritingSystemUtils::scriptTagForQCharScript(QChar::Script script)
{
    return QCHAR_SCRIPT_MAP.value(script);
}

QChar::Script KoWritingSystemUtils::qCharScriptForScriptTag(const QString &tag)
{
    return QCHAR_SCRIPT_MAP.key(tag, QChar::Script_Unknown);
}

#include <QDebug>
QMap<QString, QString> KoWritingSystemUtils::samples()
{
    QMap <QString, QString> samples;
    // Also add simplified latin sample. By doing this first, it'll fall back nicely.
    samples.insert("AaBbGg", "s_Latn");

    // Some symbol samples...
    samples.insert("\u263A\u2764\u2693\U0001F308", "s_Zsye"); // Emoji
    samples.insert("∆∅∞≠", "s_Zmth"); // Some math operators
    samples.insert("𝄞𝅘𝅥𝅮𝄿𝄻", "s_Zsym"); // Musical notes
    samples.insert("←↕↝↴", "s_Zsym"); // Arrows
    for (int i = 0; i < QFontDatabase::WritingSystemsCount; i++) {
        QFontDatabase::WritingSystem w = QFontDatabase::WritingSystem(i);
        if (w == QFontDatabase::WritingSystem::Any) continue;

        if (w == QFontDatabase::WritingSystem::Vietnamese) {
            samples.insert(QFontDatabase::writingSystemSample(QFontDatabase::Vietnamese),
                           "l_vi");
        } else {
            samples.insert(QFontDatabase::writingSystemSample(w),
                           "s_"+WRITINGSYSTEM_SCRIPT_MAP.value(w, "Zyyy"));
        }
    }

    return samples;
}

QString KoWritingSystemUtils::sampleTagForQLocale(const QLocale &locale)
{
    const QLocale vietnamese(QLocale::Vietnamese, QLocale::LatinScript, QLocale::AnyCountry);

    if (locale == vietnamese) {
        return "l_vi";
    }

    return "s_"+QLOCALE_SCRIPT_MAP.value(locale.script(), "Zyyy");
}

// There's a number of tags that are kept around for compatibility.
const QMap<QString, QString> grandFathered = {
    {"art-lojban", "jbo"},
    {"cel-gaulish", "xcg"}, // could also be xga or xtg
    {"en-GB-oed", "en-GB-oxendict"},
    {"i-ami", "ami"},
    {"i-default", ""},
    {"i-enochian", "i-enochian"},
    {"i-hak", "hak"},
    {"i-lux", "lb"},
    {"i-mingo", "i-mingo"},
    {"i-navajo", "nv"},
    {"i-pwn", "pwn"},
    {"i-tao", "tao"},
    {"i-tay", "tay"},
    {"i-tsu", "tsu"},
    {"no-bok", "nb"},
    {"no-nyn", "nn"},
    {"sgn-BE-FR", "sfb"},
    {"sgn-BE-NL", "vgt"},
    {"sgn-CH-DE", "sgg"},
    {"zh-guoyu", "cmn"},
    {"zh-hakka", "hak"},
    {"zh-min", "cdo"}, // This one could also be cpx, czo, mnp, or nan
    {"zh-min-nan", "nan"},
    {"zh-xiang", "hsn"},
};

KoWritingSystemUtils::Bcp47Locale KoWritingSystemUtils::parseBcp47Locale(const QString &locale)
{
    Bcp47Locale bcp;

    QStringList tags = grandFathered.value(locale, locale).split("-");

    if (tags.isEmpty()) return bcp;

    const QRegExp alphas("[A-Z]+", Qt::CaseInsensitive);
    const QRegExp digits("[\\d]+");

    // Language -- single primary language, followed by optional 3 letter extended tags.
    if (tags.first().size() == 2 || tags.first().size() == 3) {
        bcp.languageTags.append(tags.takeFirst().toLower());

        // extensions only happen when first tag is 2 or 3 long.
        while (!tags.isEmpty() && tags.first().size() == 3 && alphas.exactMatch(tags.first())) {
            bcp.languageTags.append(tags.takeFirst().toLower());
        }
    } else if (tags.first().size() >= 4 || tags.first().size() <= 8) {
        // 4 alpha is reserved for future use and 5-8 is also legit, but practically doesn't exist...
        bcp.languageTags.append(tags.takeFirst().toLower());
    } else if (tags.first() == "i" && tags.size() > 0) {
        QString total = tags.takeFirst();
        total += "-"+tags.takeFirst();
        bcp.languageTags.append(total.toLower());
    }

    if (tags.isEmpty()) return bcp;

    // Script -- This is an 4 letter alpha only.

    if (alphas.exactMatch(tags.first()) && tags.first().size() == 4) {
        bcp.scriptTag = tags.takeFirst().toLower();
        bcp.scriptTag = bcp.scriptTag.at(0).toUpper()+bcp.scriptTag.mid(1);
    }

    if (tags.isEmpty()) return bcp;

    // Region -- 2 letter alpha only.

    if ((alphas.exactMatch(tags.first()) && tags.first().size() == 2)
            || (digits.exactMatch(tags.first()) && tags.first().size() == 3)) {
        bcp.regionTag = tags.takeFirst().toUpper();
    }

    if (tags.isEmpty()) return bcp;

    // Variants -- [0+] alpha numerics, either between 5-8 char long, or 4 but starting with a digit.

    const QRegExp variantAlphaNumeric("\\d[a-z0-9]{3}", Qt::CaseInsensitive);

    while (!tags.isEmpty()
           && ( (tags.first().size() >= 5 && tags.first().size() <= 8)
               || (variantAlphaNumeric.exactMatch(tags.first()) && tags.first().size() == 4) )
           ) {
        bcp.variantTags.append(tags.takeFirst().toLower());
    }

    if (tags.isEmpty()) return bcp;
    // extension and private use subtags. Each starts with a single letter to indicate the extension type.

    QStringList currentExtension;
    while (!tags.isEmpty()) {
        if (!currentExtension.isEmpty() && tags.first().size() == 1) {
            if (currentExtension.first() == "x") {
                bcp.privateUseTags.append(currentExtension.join("-"));
            } else {
                bcp.extensionTags.append(currentExtension.join("-"));
            }
            currentExtension.clear();
        }
        currentExtension.append(tags.takeFirst().toLower());
    }

    if (!currentExtension.isEmpty()) {
        if (currentExtension.first() == "x") {
            bcp.privateUseTags.append(currentExtension.join("-"));
        } else {
            bcp.extensionTags.append(currentExtension.join("-"));
        }
    }

    return bcp;
}

QLocale KoWritingSystemUtils::localeFromBcp47Locale(const Bcp47Locale &locale)
{
    return QLocale(locale.toPosixLocaleFormat());
}

QLocale KoWritingSystemUtils::localeFromBcp47Locale(const QString &locale)
{
    return localeFromBcp47Locale(parseBcp47Locale(locale));
}

bool KoWritingSystemUtils::Bcp47Locale::isValid() const
{
    return !languageTags.isEmpty() && !languageTags.first().isEmpty();
}

QString KoWritingSystemUtils::Bcp47Locale::toPosixLocaleFormat() const
{
    // A Posix locale format is "language[_script][_territory][.codeset][@modifier]".

    if (!isValid()) return QString();

    QString posix;

    posix = languageTags.first();

    if (!scriptTag.isEmpty() && scriptTag.size() == 4) {
        posix.append("_");
        // Title case.
        posix.append(scriptTag);
    }

    if (!regionTag.isEmpty()) {
        posix.append("_");
        posix.append(regionTag.toUpper());
    }

    // Not writing @modifier for now...

    return posix;
}

QString KoWritingSystemUtils::Bcp47Locale::toString() const
{
    QStringList total;

    total.append(languageTags);
    if (!scriptTag.isEmpty()) {
        total.append(scriptTag);
    }
    if (!regionTag.isEmpty()) {
        total.append(regionTag);
    }
    total.append(variantTags);
    total.append(extensionTags);
    total.append(privateUseTags);

    return total.join("-");
}
