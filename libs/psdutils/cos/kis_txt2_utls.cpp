/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_txt2_utls.h"
#include <QVariantHash>
#include <QVariant>
#include <QVariantList>
#include <QDebug>
#include <QRectF>

QVariantHash uncompressColor(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;
    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        if (key == "/0") {
            QVariantHash color = val.toHash();
            QVariantHash newColor;
            Q_FOREACH(QString key, color.keys()) {
                if (key == "/1") {
                    newColor.insert("/Values", color.value(key));
                } else if (key == "/0") {
                    newColor.insert("/Type", color.value(key));
                } else {
                    newColor.insert(key, color.value(key));
                }
            }
            newObject.insert("/Color", newColor);
        } else if (key == "/99") {
            newObject.insert("/StreamTag", val);
        } else {
            newObject.insert(key, val);
        }
    }
    return newObject;
}

QVariantHash uncompressStyleSheetFeatures(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    const QMap<QString, QString> keyList{
        {"/0", "/Font"},
        {"/1", "/FontSize"},
        {"/2", "/FauxBold"},
        {"/3", "/FauxItalic"},
        {"/4", "/AutoLeading"},

        {"/5", "/Leading"},
        {"/6", "/HorizontalScale"},
        {"/7", "/VerticalScale"},
        {"/8", "/Tracking"},
        {"/9", "/BaselineShift"},

        {"/10", "/CharacterRotation"},
        {"/11", "/AutoKern"},
        {"/12", "/FontCaps"},
        {"/13", "/FontBaseline"},
        {"/14", "/FontOTPosition"},

        {"/15", "/StrikethroughPosition"},
        {"/16", "/UnderlinePosition"},
        {"/17", "/UnderlineOffset"},
        {"/18", "/Ligatures"},
        {"/19", "/DiscretionaryLigatures"},

        {"/20", "/ContextualLigatures"},
        {"/21", "/AlternateLigatures"},
        {"/22", "/OldStyle"},
        {"/23", "/Fractions"},
        {"/24", "/Ordinals"},

        {"/25", "/Swash"},
        {"/26", "/Titling"},
        {"/27", "/ConnectionForms"},
        {"/28", "/StylisticAlternates"},
        {"/29", "/Ornaments"},

        {"/30", "/FigureStyle"},
        {"/31", "/ProportionalMetrics"},
        {"/32", "/Kana"},
        {"/33", "/Italics"},
        {"/34", "/Ruby"},

        {"/35", "/BaselineDirection"},
        {"/36", "/Tsume"},
        {"/37", "/StyleRunAlignment"},
        {"/38", "/Language"},
        {"/39", "/JapaneseAlternateFeature"},

        {"/40", "/EnableWariChu"},
        {"/41", "/WariChuLineCount"},
        {"/42", "/WariChuLineGap"},
        {"/43", "/WariChuSubLineAmount"},
        {"/44", "/WariChuWidowAmount"},

        {"/45", "/WariChuOrphanAmount"},
        {"/46", "/WariChuJustification"},
        {"/47", "/TCYUpDownAdjustment"},
        {"/48", "/TCYLeftRightAdjustment"},
        {"/49", "/LeftAki"},

        {"/50", "/RightAki"},
        {"/51", "/JiDori"},
        {"/52", "/NoBreak"},
        //{"/53", "/FillColor"},
        //{"/54", "/StrokeColor"},

        {"/55", "/Blend"},
        {"/56", "/FillFlag"},
        {"/57", "/StrokeFlag"},
        {"/58", "/FillFirst"},
        {"/59", "/FillOverPrint"},

        {"/60", "/StrokeOverPrint"},
        {"/61", "/LineCap"},
        {"/62", "/LineJoin"},
        {"/63", "/LineWidth"},
        {"/64", "/MiterLimit"},

        {"/65", "/LineDashOffset"},
        {"/66", "/LineDashArray"},
        {"/67", "/Type"},
        {"/68", "/Kashidas"},
        {"/69", "/DirOverride"},

        {"/70", "/DigitSet"},
        {"/71", "/DiacVPos"},
        {"/72", "/DiacXOffset"},
        {"/73", "/DiacYOffset"},
        {"/74", "/OverlapSwash"},

        {"/75", "/JustificationAlternates"},
        {"/76", "/StretchedAlternates"},
        {"/77", "/FillVisibleFlag"},
        {"/78", "/StrokeVisibleFlag"},
        //{"/79", "/FillBackgroundColor"},

        {"/80", "/FillBackgroundFlag"},
        {"/81", "/UnderlineStyle"},
        {"/82", "/DashedUnderlineGapLength"},
        {"/83", "/DashedUnderlineDashLength"},
        {"/84", "/SlashedZero"},

        {"/85", "/StylisticSets"},
        {"/86", "/CustomFeature"},
        {"/87", "/MarkYDistFromBaseline"},
        {"/88", "/AutoMydfb"},
        {"/89", "/RefFontSize"},

        {"/90", "/FontSizeRefType"},
        {"/92", "/MagicLineGap"},
        {"/92", "/MagicWordGap"},
    };

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        if (key == "/53") {
            newObject.insert("/FillColor", uncompressColor(val.toHash()));
        } else if (key == "/54") {
            newObject.insert("/StrokeColor", uncompressColor(val.toHash()));
        } else if (key == "/79") {
            newObject.insert("/FillBackgroundColor", uncompressColor(val.toHash()));
        } else if (keyList.keys().contains(key)) {
            newObject.insert(keyList.value(key), val);
        } else {
            newObject.insert(key, val);
        }
    }
    return newObject;
}

QVariantHash uncompressParagraphSheetFeatures(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    const QMap<QString, QString> keyList{
        {"/0", "/Justification"},
        {"/1", "/FirstLineIndent"},
        {"/2", "/StartIndent"},
        {"/3", "/EndIndent"},
        {"/4", "/SpaceBefore"},

        {"/5", "/SpaceAfter"},
        {"/6", "/DropCaps"},
        {"/7", "/AutoLeading"},
        {"/8", "/LeadingType"},
        {"/9", "/AutoHyphenate"},

        {"/10", "/HyphenatedWordSize"},
        {"/11", "/PreHyphen"},
        {"/12", "/PostHyphen"},
        {"/13", "/ConsecutiveHyphens"},
        {"/14", "/Zone"},

        {"/15", "/HyphenateCapitalized"},
        {"/16", "/HyphenationPreference"},
        {"/17", "/WordSpacing"},
        {"/18", "/LetterSpacing"},
        {"/19", "/GlyphSpacing"},

        {"/20", "/SingleWordJustification"},
        {"/21", "/Hanging"},
        {"/22", "/AutoTCY"},
        {"/23", "/KeepTogether"},
        {"/24", "/BurasagariType"},

        {"/25", "/KinsokuOrder"},
        {"/26", "/Kinsoku"},
        {"/27", "/KurikaeshiMojiShori"},
        {"/28", "/MojiKumiTable"},
        {"/29", "/EveryLineComposer"},

        {"/30", "/TabStops"},
        {"/31", "/DefaultTabWidth"},
        //{"/32", "/DefaultStyle"},
        {"/33", "/ParagraphDirection"},
        {"/34", "/JustificationMethod"},

        {"/35", "/ComposerEngine"},
        {"/36", "/ListStyle"},
        {"/37", "/ListTier"},
        {"/38", "/ListSkip"},
        {"/39", "/ListOffset"},

        {"/40", "/KashidaWidth"}
    };

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        if (key == "/32") {
            newObject.insert("/DefaultStyle", uncompressStyleSheetFeatures(val.toHash()));
        } else if (keyList.keys().contains(key)) {
            newObject.insert(keyList.value(key), val);
        } else {
            newObject.insert(key, val);
        }
    }
    return newObject;
}

QVariantHash uncompressKeysStyleSheetSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    QVariantList resources = object.value("/0").toList();
    QVariantList newResources;

    Q_FOREACH(QVariant val, resources) {
        QVariantHash resource = val.toHash().value("/0").toHash();
        QVariantHash newResource;
        Q_FOREACH(QString key, resource.keys()) {
            QMap<QString, QString> keyList {{"/0", "/Name"}, {"/5", "/Parent"}, {"/97", "/UUID"}};
            QVariant rdVal = resource.value(key);
            if (key == "/6") {
                newResource.insert("/Features", uncompressStyleSheetFeatures(rdVal.toHash()));
            } else if (keyList.keys().contains(key)) {
                newResource.insert(keyList.value(key), rdVal);
            } else {
                newResource.insert(key, rdVal);
            }
        }

        newResources.append(QVariantHash({{"/Resource", newResource}}));
    }
    newObject.insert("/Resources", newResources);
    return newObject;
}

QVariantHash uncompressKeysParagraphSheetSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;
    QVariantList resources = object.value("/0").toList();
    QVariantList newResources;

    const QMap<QString, QString> keyList {{"/0", "/Name"}, {"/6", "/Parent"}, {"/97", "/UUID"}};

    Q_FOREACH(QVariant val, resources) {
        QVariantHash resource = val.toHash().value("/0").toHash();
        QVariantHash newResource;
        Q_FOREACH(QString key, resource.keys()) {
            QVariant rdVal = resource.value(key);
            if (key == "/5") {
                newResource.insert("/Features", uncompressParagraphSheetFeatures(rdVal.toHash()));
            } else if (keyList.keys().contains(key)) {
                newResource.insert(keyList.value(key), rdVal);
            } else {
                newResource.insert(key, rdVal);
            }
        }

        newResources.append(QVariantHash({{"/Resource", newResource}}));
    }
    newObject.insert("/Resources", newResources);
    return newObject;
}

QVariantHash uncompressTextFrameData(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;
    const QMap<QString, QString> keyList {
        {"/0", "/Type"},
        {"/1", "/LineOrientation"},
        {"/2", "/FrameMatrix"},
        {"/3", "/RowCount"},
        {"/4", "/ColumnCount"},

        {"/5", "/RowMajorOrder"},
        {"/6", "/TextOnPathTRange"},
        {"/7", "/RowGutter"},
        {"/8", "/ColumnGutter"},
        {"/9", "/Spacing"},

        {"/10", "/FirstBaseAlignment"},
       // {"/11", "/PathData"},
        //{"/12", ""},
        {"/13", "/VerticalAlignment"},
    };

    const QMap<QString, QString> pathDataList {
        {"/0", "/Flip"},
        {"/1", "/Effect"},
        {"/2", "/Alignment"},
        {"/4", "/Spacing"},
        {"/18", "/Spacing2"},
    };

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        if (key == "/11") {
            QVariantHash data = val.toHash();
            QVariantHash newData;
            Q_FOREACH(QString key2, data.keys()) {
                if (pathDataList.keys().contains(key2)) {
                    newData.insert(pathDataList.value(key2), data.value(key2));
                } else {
                    newData.insert(key2, data.value(key2));
                }
            }
            newObject.insert("/PathData", newData);
        } else if (keyList.keys().contains(key)) {
            newObject.insert(keyList.value(key), val);
        } else {
            newObject.insert(key, val);
        }
    }
    return newObject;
}

QVariantHash uncompressKeysTextFrameSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    QVariantList resources = object.value("/0").toList();
    QVariantList newResources;
    const QMap<QString, QString> keyList {
        {"/0", "/Position"},
        {"/1", "/Bezier"},
        //{"/2", "/Data"},
        {"/97", "/UUID"}
    };

    Q_FOREACH(QVariant val, resources) {
        QVariantHash resource = val.toHash().value("/0").toHash();
        QVariantHash newResource;
        Q_FOREACH(QString key2, resource.keys()) {
            QVariant rdVal = resource.value(key2);
            if (key2 == "/5") {
                newResource.insert("/Features", uncompressStyleSheetFeatures(rdVal.toHash()));
            } else if (key2 == "/1") {
                QVariant pList = rdVal.toHash().value("/0");

                newResource.insert("/Bezier", QVariantHash({{"/Points", pList}}));
            } else if (key2 == "/2") {
                newResource.insert("/Data", uncompressTextFrameData(rdVal.toHash()));
            } else if (keyList.keys().contains(key2)) {
                newResource.insert(keyList.value(key2), rdVal);
            } else {
                newResource.insert(key2, rdVal);
            }
        }

        newResources.append(QVariantHash({{"/Resource", newResource}}));
    }
    newObject.insert("/Resources", newResources);
    return newObject;
}

QVariantHash uncompressKeysKinsokuSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    QVariantList resources = object.value("/0").toList();
    QVariantList newResources;

    const QMap<QString, QString> keyList {{"/0", "/Name"}, {"/5", "/Data"}};
    const QMap<QString, QString> idKeyList {{"/0", "/NoStart"}, {"/1", "/NoEnd"}, {"/2", "/Keep"}, {"/3", "/Hanging"}, {"/4", "/PredefinedTag"}};


    Q_FOREACH(QVariant val, resources) {
        QVariantHash resource = val.toHash().value("/0").toHash();
        QVariantHash newResource;
        Q_FOREACH(QString key, resource.keys()) {
            if (key == "/5") {
                QVariantHash id = resource.value(key).toHash();
                QVariantHash newId;
                Q_FOREACH(QString key2, id.keys()) {
                    QVariant idVal = id.value(key2);
                    if (idKeyList.keys().contains(key2)) {
                        newId.insert(idKeyList.value(key2), idVal);
                    } else {
                        newId.insert(key2, idVal);
                    }
                }
                newResource.insert("/Data", newId);
            } else if (keyList.keys().contains(key)) {
                newResource.insert(keyList.value(key), resource.value(key));
            } else {
                newResource.insert(key, resource.value(key));
            }
        }

        newResources.append(QVariantHash({{"/Resource", newResource}}));
    }
    newObject.insert("/Resources", newResources);
    return newObject;
}

QVariantHash uncompressKeysMojiKumiTableSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        newObject.insert(key, val);
    }
    return newObject;
}

QVariantHash uncompressKeysMojiKumiCodeToClassSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        newObject.insert(key, val);
    }
    return newObject;
}

QVariantHash uncompressKeysFontSet(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    QVariantList resources = object.value("/0").toList();
    QVariantList newResources;

    const QMap<QString, QString> keyList {{"/99", "/StreamTag"}, {"/97", "/UUID"}};
    const QMap<QString, QString> idKeyList {{"/0", "/Name"}, {"/2", "/Type"}, {"/4", "/MMAxis"}, {"/5", "/VersionString"}};


    Q_FOREACH(QVariant val, resources) {
        QVariantHash resource = val.toHash().value("/0").toHash();
        QVariantHash newResource;
        Q_FOREACH(QString key, resource.keys()) {
            if (key == "/0") {
                QVariantHash id = resource.value(key).toHash();
                QVariantHash newId;
                Q_FOREACH(QString key2, id.keys()) {
                    QVariant idVal = id.value(key2);
                    if (idKeyList.keys().contains(key2)) {
                        newId.insert(idKeyList.value(key2), idVal);
                    } else {
                        newId.insert(key2, idVal);
                    }
                }
                newResource.insert("/Identifier", newId);
            } else if (keyList.keys().contains(key)) {
                newResource.insert(keyList.value(key), resource.value(key));
            } else {
                newResource.insert(key, resource.value(key));
            }
        }

        newResources.append(QVariantHash({{"/Resource", newResource}}));
    }
    newObject.insert("/Resources", newResources);
    return newObject;
}

QVariantHash uncompressKeysDocumentResources(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);

         if (key == "/1") {
             newObject.insert("/FontSet", uncompressKeysFontSet(val.toHash()));
         } else if (key == "/2") {
             newObject.insert("/MojiKumiCodeToClassSet", uncompressKeysMojiKumiCodeToClassSet(val.toHash()));
         } else if (key == "/3") {
             newObject.insert("/MojiKumiTableSet", uncompressKeysMojiKumiTableSet(val.toHash()));
         } else if (key == "/4") {
             newObject.insert("/KinsokuSet", uncompressKeysKinsokuSet(val.toHash()));
         } else if (key == "/5") {
             newObject.insert("/StyleSheetSet", uncompressKeysStyleSheetSet(val.toHash()));
         } else if (key == "/6") {
             newObject.insert("/ParagraphSheetSet", uncompressKeysParagraphSheetSet(val.toHash()));
         } else if (key == "/8") {
             newObject.insert("/TextFrameSet", uncompressKeysTextFrameSet(val.toHash()));
         } else if (key == "/9") {
             newObject.insert("/ListStyleSet", val);
         } else {
             newObject.insert(key, val);
         }
    }
    return newObject;
}

/*------- Document Objects ----------*/

QVariantHash uncompressKeysTextModel(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    const QMap<QString, QString> runStyleKeyList {{"/0", "/Name"}, {"/5", "/Parent"}, {"/97", "/UUID"}};

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);

         if (key == "/0") {
             newObject.insert("/Text", val);
         } else if (key == "/5") {
             QVariantList array = val.toHash().value("/0").toList();
             QVariantList newArray;

             Q_FOREACH(QVariant run, array) {
                 QVariantHash runDataSheet = run.toHash().value("/0").toHash().value("/0").toHash();
                 QVariantHash newDataSheet;

                 Q_FOREACH(QString key2, runDataSheet.keys()) {
                     QVariant rdVal = runDataSheet.value(key2);
                     if (key2 == "/5") {
                         newDataSheet.insert("/Features", uncompressParagraphSheetFeatures(rdVal.toHash()));
                     } else if (key2 == "/6") {
                         newDataSheet.insert("/Parent", rdVal);
                     } else if (runStyleKeyList.keys().contains(key)) {
                         newDataSheet.insert(runStyleKeyList.value(key2), rdVal);
                     } else {
                         newDataSheet.insert(key2, rdVal);
                     }
                 }

                 QVariantHash newSheet =  {{"/ParagraphSheet", newDataSheet}};
                 QVariantHash newRunData = {{"/RunData", newSheet}};
                 newRunData.insert("/Length", run.toHash().value("/1"));
                 newArray.append(newRunData);
             }
             QVariantHash arrayParent = {{"/RunArray", newArray}};
             newObject.insert("/ParagraphRun", arrayParent);
         } else if (key == "/6") {
             QVariantList array = val.toHash().value("/0").toList();
             QVariantList newArray;

             Q_FOREACH(QVariant run, array) {
                 QVariantHash runDataSheet = run.toHash().value("/0").toHash().value("/0").toHash();
                 QVariantHash newDataSheet;

                 Q_FOREACH(QString key2, runDataSheet.keys()) {
                     QVariant rdVal = runDataSheet.value(key2);
                     if (key2 == "/6") {
                         newDataSheet.insert("/Features", uncompressStyleSheetFeatures(rdVal.toHash()));
                     } else if (key2 == "/5") {
                         newDataSheet.insert("/Parent", rdVal);
                     } else if (runStyleKeyList.keys().contains(key)) {
                         newDataSheet.insert(runStyleKeyList.value(key2), rdVal);
                     } else {
                         newDataSheet.insert(key2, rdVal);
                     }
                 }

                 QVariantHash newSheet =  {{"/StyleSheet", newDataSheet}};
                 QVariantHash newRunData = {{"/RunData", newSheet}};
                 newRunData.insert("/Length", run.toHash().value("/1"));
                 newArray.append(newRunData);
             }
             QVariantHash arrayParent = {{"/RunArray", newArray}};
             newObject.insert("/StyleRun", arrayParent);
         } else if (key == "/10") {
             newObject.insert("/StorySheet", val);
         } else {
             newObject.insert(key, val);
         }
    }
    return newObject;
}

QVariantHash uncompressStrikeDef(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;
    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);

         if (key == "/99") {
             newObject.insert("/StreamTag", val);
         } else if (key == "/0") {
             newObject.insert("/Bounds", val);
         } else if (key == "/1") {
             newObject.insert("/Transform", val);
         } else if (key == "/5") {
             newObject.insert("/ChildProcession", val);
         } else if (key == "/6") {
             QVariantList array = val.toList();
             QVariantList newArray;
             Q_FOREACH(QVariant entry, array) {
                 newArray.append(uncompressStrikeDef(entry.toHash()));
             }
             newObject.insert("/Children", newArray);
         } else {
             newObject.insert(key, val);
         }
    }
    return newObject;
}

QVariantHash uncompressKeysTextView(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;
    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);

         if (key == "/0") {
             QVariantList array = val.toList();
             QVariantList newArray;
             Q_FOREACH(QVariant entry, array) {
                 QVariant resource = entry.toHash().value("/0");

                newArray.append(QVariantHash({{"/Resource", resource}}));
             }

             newObject.insert("/Frames", newArray);
         } else if (key == "/2") {
             QVariantList array = val.toList();
             QVariantList newArray;
             Q_FOREACH(QVariant entry, array) {
                 newArray.append(uncompressStrikeDef(entry.toHash()));
             }
             newObject.insert("/Strikes", newArray);
         } else {
             newObject.insert(key, val);
         }
    }
    return newObject;
}

QVariantHash uncompressKeysTextObject(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);

         if (key == "/0") {
             newObject.insert("/Model", uncompressKeysTextModel(val.toHash()));
         } else if (key == "/1") {
             newObject.insert("/View", uncompressKeysTextView(val.toHash()));
         } else {
             newObject.insert(key, val);
         }
    }
    return newObject;
}

QVariantHash uncompressSmartQuoteSettings(const QVariantHash object) {
    QVariantHash newObject;
    const QMap<QString, QString> keyList {
        {"/0", "/Language"},
        {"/1", "/OpenDoubleQuote"},
        {"/2", "/CloseDoubleQuote"},
        {"/3", "/OpenSingleQuote"},
        {"/4", "/CloseSingleQuote"},

    };

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        if (keyList.keys().contains(key)) {
            newObject.insert(keyList.value(key), val);
        } else {
            newObject.insert(key, val);
        }
    }
    return newObject;
}

QVariantHash uncompressHiddenGlyphSettings(const QVariantHash object) {
    QVariantHash newObject;

    if (object.keys().contains("/0")) {
        newObject.insert("/AlternateGlyphFont", object.value("/0"));
    }
    if (object.keys().contains("/1")) {
        QVariantList array = object.value("/1").toList();
        QVariantList newArray;
        Q_FOREACH(QVariant entry, array) {
            QVariantHash newEntry;
            newObject.insert("/WhitespaceCharacter", newEntry.value("/0"));
            newObject.insert("/AlternateCharacter", newEntry.value("/1"));
            newArray.append(newObject);
        }
        newObject.insert("/WhitespaceCharacterMapping", newArray);
    }

    return newObject;
}

QVariantHash uncompressKeysDocumentSettings(const QVariantHash object) {
    QVariantHash newObject;
    const QMap<QString, QString> keyList {
        //{"/0", "/HiddenGlyphFont"},
        {"/1", "/NormalStyleSheet"},
        {"/2", "/NormalParagraphSheet"},
        {"/3", "/SuperscriptSize"},
        {"/4", "/SuperscriptPosition"},

        {"/5", "/SubscriptSize"},
        {"/6", "/SubscriptPosition"},
        {"/7", "/SmallCapSize"},
        {"/8", "/UseSmartQuotes"},
        //{"/9", "/SmartQuoteSets"},

        //{"/10", ""},
        {"/11", "/GreekingSize"},
        //{"/12", ""},
        //{"/13", ""},
        //{"/14", ""},

        {"/15", "/LinguisticSettings"},
        {"/16", "/UseSmartLists"},
        {"/17", "/DefaultStoryDir"},
    };

    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);
        if (key == "/0") {
            newObject.insert("/HiddenGlyphFont", uncompressHiddenGlyphSettings(val.toHash()));
        } else if (key == "/9") {
            QVariantList array = val.toList();
            QVariantList newArray;
            Q_FOREACH(QVariant entry, array) {
                newArray.append(uncompressSmartQuoteSettings(entry.toHash()));
            }
            newObject.insert("/SmartQuoteSets", newArray);
        } else if (keyList.keys().contains(key)) {
            newObject.insert(keyList.value(key), val);
        } else {
            newObject.insert(key, val);
        }
    }

    return newObject;
}

QVariantHash uncompressKeysDocumentObjects(const QVariantHash object) {
    //qDebug() << Q_FUNC_INFO;
    QVariantHash newObject;
    Q_FOREACH(QString key, object.keys()) {
        QVariant val = object.value(key);

         if (key == "/0") {
             newObject.insert("/DocumentSettings", uncompressKeysDocumentSettings(val.toHash()));
         } else if (key == "/1") {
             QVariantList array = val.toList();
             QVariantList newArray;
             Q_FOREACH(QVariant entry, array) {
                 newArray.append(uncompressKeysTextObject(entry.toHash()));
             }
             newObject.insert("/TextObjects", newArray);
         } else if (key == "/2") {
             newObject.insert("/OriginalNormalStyleFeatures", uncompressStyleSheetFeatures(val.toHash()));
         } else if (key == "/3") {
             newObject.insert("/OriginalNormalParagraphFeatures", uncompressParagraphSheetFeatures(val.toHash()));
         } else {
             newObject.insert(key, val);
         }
    }

    return newObject;
}

QVariantHash KisTxt2Utils::uncompressKeys(QVariantHash doc)
{

    QVariantHash newDoc;
    Q_FOREACH(QString key, doc.keys()) {
        if (key == "/0") {
            newDoc.insert("/DocumentResources", uncompressKeysDocumentResources(doc.value(key).toHash()));
        } else if (key == "/1") {
            newDoc.insert("/DocumentObjects", uncompressKeysDocumentObjects(doc.value(key).toHash()));
        } else {
            newDoc.insert(key, doc.value(key));
        }
    }
    return newDoc;
}

//-------------- Default Txt2 ----------------//

static QVariantHash defaultFill {
    { "/StreamTag", "/SimplePaint"},
    {
        "/Color", QVariantHash{{"/Type", 1}, {"/Values", QVariantList({1.0, 0.0, 0.0, 0.0})}}
    }
};

static QVariantHash defaultBGFill {
    { "/StreamTag", "/SimplePaint"},
    {
        "/Color", QVariantHash{{"/Type", 1}, {"/Values", QVariantList({1.0, 1.0, 1.0, 0.0})}}
    }
};

static QVariantHash defaultStyle {
    {"/Font", 1},
    {"/FontSize", 12.0},
    {"/FauxBold", false},
    {"/FauxItalic", false},
    {"/AutoLeading", true},

    {"/Leading", 0.0},
    {"/HorizontalScale", 1.0},
    {"/VerticalScale", 1.0},
    {"/Tracking", 0.0},
    {"/BaselineShift", 0.0},

    {"/CharacterRotation", 0.0},
    {"/AutoKern", 1},
    {"/FontCaps", 0},
    {"/FontBaseline", 0},
    {"/FontOTPosition", 0},

    {"/StrikethroughPosition", 0},
    {"/UnderlinePosition", 0},
    {"/UnderlineOffset", 0.0},
    {"/Ligatures", true},
    {"/DiscretionaryLigatures", false},

    {"/ContextualLigatures", false},
    {"/AlternateLigatures", false},
    {"/OldStyle", false},
    {"/Fractions", false},
    {"/Ordinals", false},

    {"/Swash", false},
    {"/Titling", false},
    {"/ConnectionForms", false},
    {"/StylisticAlternates", false},
    {"/Ornaments", false},

    {"/FigureStyle", 0},
    {"/ProportionalMetrics", false},
    {"/Kana", false},
    {"/Italics", false},
    {"/Ruby", false},

    {"/BaselineDirection", 2},
    {"/Tsume", 0.0},
    {"/StyleRunAlignment", 0},
    {"/Language", 0},
    {"/JapaneseAlternateFeature", 0},

    {"/EnableWariChu", false},
    {"/WariChuLineCount", 2},
    {"/WariChuLineGap", 0},
    {"/WariChuSubLineAmount", QVariantHash({{"/WariChuSubLineScale", 0.5}})},
    {"/WariChuWidowAmount", 2},

    {"/WariChuOrphanAmount", 2},
    {"/WariChuJustification", 7},
    {"/TCYUpDownAdjustment", 0.0},
    {"/TCYLeftRightAdjustment", 0.0},
    {"/LeftAki", -1.0},

    {"/RightAki", -1.0},
    {"/JiDori", 0},
    {"/NoBreak", false},
    {"/FillColor", defaultFill},
    {"/StrokeColor", defaultFill},

    {"/Blend",  QVariantHash({{"/StreamTag", "/SimpleBlender"}})},
    {"/FillFlag", true},
    {"/StrokeFlag", false},
    {"/FillFirst", false},

    {"/StrokeOverPrint", false},
    {"/LineCap", 0},
    {"/LineJoin", 0},
    {"/LineWidth", 1.0},
    {"/MiterLimit", 4.0},

    {"/LineDashOffset", 0.0},
    {"/LineDashArray", QVariantList()},
    {"/Type", QVariantList()},
    {"/Kashidas", 0},
    {"/DirOverride", 0},

    {"/DigitSet", 0},
    {"/DiacVPos", 0.0},
    {"/DiacXOffset", 0.0},
    {"/DiacYOffset", 0.0},
    {"/OverlapSwash", false},

    {"/JustificationAlternates", false},
    {"/StretchedAlternates", false},
    {"/FillVisibleFlag", true},
    {"/StrokeVisibleFlag", true},
    {"/FillBackgroundColor", defaultBGFill},

    {"/FillBackgroundFlag", false},
    {"/UnderlineStyle", 0},
    {"/DashedUnderlineGapLength", 3.0},
    {"/DashedUnderlineDashLength", 3.0},
    {"/SlashedZero", false},

    {"/StylisticSets", 0},
    {"/CustomFeature", QVariantHash({{"/StreamTag", "/SimpleCustomFeature"}})},
    {"/MarkYDistFromBaseline", 100.0},
};

static QVariantHash defaultParagraph {
    {"/Justification", 0},
    {"/FirstLineIndent", 0.0},
    {"/StartIndent", 0.0},
    {"/EndIndent", 0.0},
    {"/SpaceBefore", 0.0},

    {"/SpaceAfter", 0.0},
    {"/DropCaps", 1},
    {"/AutoLeading", 1.2},
    {"/LeadingType", 0},
    {"/AutoHyphenate", true},

    {"/HyphenatedWordSize", 6},
    {"/PreHyphen", 2},
    {"/PostHyphen", 2},
    {"/ConsecutiveHyphens", 0},
    {"/Zone", 36.0},

    {"/HyphenateCapitalized", true},
    {"/HyphenationPreference", 0.5},
    {"/WordSpacing", QVariantList({0.8, 1.0, 1.3})},
    {"/LetterSpacing", QVariantList({0.0, 0.0, 0.0})},
    {"/GlyphSpacing", QVariantList({1.0, 1.0, 1.0})},

    {"/SingleWordJustification", 6},
    {"/Hanging", false},
    {"/AutoTCY", 0},
    {"/KeepTogether", true},
    {"/BurasagariType", 0},

    {"/KinsokuOrder", 0},
    {"/Kinsoku", false},
    {"/KurikaeshiMojiShori", "/nil"},
    {"/MojiKumiTable", "/nil"},
    {"/EveryLineComposer", false},

    {"/TabStops", QVariantHash()},
    {"/DefaultTabWidth", 36.0},
    {"/DefaultStyle", QVariantHash()},
    {"/ParagraphDirection", 0},
    {"/JustificationMethod", 7},

    {"/ComposerEngine", 1},
    {"/ListStyle", "/nil"},
    {"/ListTier", 0},
    {"/ListSkip", false},
    {"/ListOffset", 0},

    {"/KashidaWidth", 2}
};

static QVariantHash kinsokuNone {
    {"/NoStart", ""},
    {"/NoEnd", ""},
    {"/Keep", ""},
    {"/Hanging", ""},
    {"/PredefinedTag", 0}
};

static QVariantHash kinsokuHard {
    {"/NoStart", "!),.:;?]}¢—’”‰℃℉、。々〉》」』】〕ぁぃぅぇぉっゃゅょゎ゛゜ゝゞァィゥェォッャュョヮヵヶ・ーヽヾ！％），．：；？］｝"},
    {"/NoEnd", "([{£§‘“〈《「『【〒〔＃＄（＠［｛￥"},
    {"/Keep", "—‥…"},
    {"/Hanging", "、。，．"},
    {"/PredefinedTag", 1}
};

static QVariantHash kinsokuSoft {
    {"/NoStart", "’”、。々〉》」』】〕ゝゞ・ヽヾ！），．：；？］｝"},
    {"/NoEnd", "‘“〈《「『【〔（［｛"},
    {"/Keep", "—‥…"},
    {"/Hanging", "、。，．"},
    {"/PredefinedTag", 2}
};

QVariantHash KisTxt2Utils::defaultTxt2()
{
    QVariantHash doc;

    QVariantHash documentResources;
    QVariantHash documentObjects;

    QVariantHash fontSet;
    QVariantList fontResources;

    QVariantHash fontInvis {
        {"/Name", "AdobeInvisFont"},
        {"/Type", 0}
    };

    QVariantHash fontMyriad {
        {"/Name", "MyriadPro-Regular"},
        {"/Type", 0},
        {"/Version", "Version 2.115;PS 2.000;hotconv 1.0.81;makeotf.lib2.5.63406"}
    };
    QVariantHash fR = {
        {"/Resource", QVariantHash({{"/Identifier", fontMyriad}, {"/StreamTag", "/CoolTypeFont"}})}
    };
    fontResources.append(fR);
    fR = {
        {"/Resource", QVariantHash({{"/Identifier", fontInvis}, {"/StreamTag", "/CoolTypeFont"}})}
    };
    fontResources.append(fR);

    fontSet.insert("/Resources", fontResources);
    documentResources.insert("/FontSet", fontSet);
    QVariantHash kinsokuSet;
    QVariantList kinsokuResources;
    QVariantList kinsokuDisplay;
    QVariantHash kin =  QVariantHash ({{"/Resource", QVariantHash({ {"/Name", "None"}, {"/Data", kinsokuNone}})}});
    kinsokuResources.append(kin);
    kinsokuDisplay.append(QVariantHash({{"/Resource", 0}}));
    kin =  QVariantHash ({{"/Resource", QVariantHash({ {"/Name", "PhotoshopKinsokuHard"}, {"/Data", kinsokuHard}})}});
    kinsokuResources.append(kin);
    kinsokuDisplay.append(QVariantHash({{"/Resource", 1}}));
    kin =  QVariantHash ({{"/Resource", QVariantHash({ {"/Name", "PhotoshopKinsokuSoft"}, {"/Data", kinsokuSoft}})}});
    kinsokuResources.append(kin);
    kinsokuDisplay.append(QVariantHash({{"/Resource", 2}}));
    kin =  QVariantHash ({{"/Resource", QVariantHash({ {"/Name", "Hard"}, {"/Data", kinsokuHard}})}});
    kinsokuResources.append(kin);
    kinsokuDisplay.append(QVariantHash({{"/Resource", 3}}));
    kin =  QVariantHash ({{"/Resource", QVariantHash({ {"/Name", "Soft"}, {"/Data", kinsokuSoft}})}});
    kinsokuResources.append(kin);
    kinsokuDisplay.append(QVariantHash({{"/Resource", 4}}));
    kinsokuSet.insert("/Resources", kinsokuResources);
    kinsokuSet.insert("/DisplayList", kinsokuDisplay);
    documentResources.insert("/KinsokuSet", kinsokuSet);

    //QVariantHash listStyleSet;
    //documentResources.insert("/ListStyleSet", listStyleSet);
    QVariantHash mojiKumiCodeToClassSet = {
        {"/Resources", QVariantList({
             QVariantHash({
                 {"/Resource", QVariantHash({
                      {"/Name", ""}
                  })
                 }
             })
         })},
        {"/DisplayList", QVariantList({QVariantHash({{"/Resource", 0}})})}
    };
    documentResources.insert("/MojiKumiCodeToClassSet", mojiKumiCodeToClassSet);

    QHash<QString, int> mojilist {
        {"Photoshop6MojiKumiSet4", 2},
        {"Photoshop6MojiKumiSet3", 4},
        {"Photoshop6MojiKumiSet2", 3},
        {"Photoshop6MojiKumiSet1", 1},
        {"YakumonoHankaku", 1},
        {"GyomatsuYakumonoHankaku", 3},
        {"GyomatsuYakumonoZenkaku", 4},
        {"YakumonoZenkaku", 2},
    };

    QVariantHash mojiKumiTableSet;

    QVariantList mojiResources;
    QVariantList mojiDisplay;

    Q_FOREACH(const QString key, mojilist.keys()) {
        mojiDisplay.append(QVariantHash({{"/Resource", mojiDisplay.size()}}));
        QVariantHash mojikumiTable = {
            {"/Name", key},
            {"/Members", QVariantHash({
                 {"/CodeToClass", 0},
                 {"/PredefinedTag", mojilist.value(key)}

             })
            },
        };

        mojiResources.append(mojikumiTable);
    }

    mojiKumiTableSet.insert("/Resources", mojiResources);
    mojiKumiTableSet.insert("/DisplayList", mojiDisplay);
    documentResources.insert("/MojiKumiTableSet", mojiKumiTableSet);

    QVariantHash paragraphSheetSet = {
        {"/Resources", QVariantList({
             QVariantHash({
                 {"/Resource", QVariantHash({
                      {"/Name", "Normal RGB"},
                      {"/Features", defaultParagraph}
                  })
                 }
             })
         })},
        {"/DisplayList", QVariantList({QVariantHash({{"/Resource", 0}})})}
    };
    documentResources.insert("/ParagraphSheetSet", paragraphSheetSet);
    QVariantHash styleSheetSet = {
        {"/Resources", QVariantList({
             QVariantHash({
                 {"/Resource", QVariantHash({
                      {"/Name", "Normal RGB"},
                      {"/Features", defaultStyle}
                  })
                 }
             })
         })},
        {"/DisplayList", QVariantList({QVariantHash({{"/Resource", 0}})})}
    };
    documentResources.insert("/StyleSheetSet", styleSheetSet);

    QVariantHash textFrameSet;
    textFrameSet.insert("/Resources", QVariantList());
    documentResources.insert("/TextFrameSet", textFrameSet);

    // Document settings ----------
    QVariantHash DocumentSettings;
    DocumentSettings.insert("/DefaultStoryDir", 0);
    // smart quote?
    // alternate glyph?
    DocumentSettings.insert("/SubscriptPosition", 0.333);
    DocumentSettings.insert("/SubscriptSize", 0.583);
    DocumentSettings.insert("/SuperscriptPosition", 0.333);
    DocumentSettings.insert("/SuperscriptSize", 0.583);
    DocumentSettings.insert("/SmallCapSize", 0.7);
    DocumentSettings.insert("/NormalParagraphSheet", 0);
    DocumentSettings.insert("/NormalStyleSheet", 0);
    documentObjects.insert("/DocumentSettings", DocumentSettings);
    QVariantHash OriginalNormalParagraphFeatures = defaultParagraph;
    documentObjects.insert("/OriginalNormalParagraphFeatures", OriginalNormalParagraphFeatures);
    QVariantHash OriginalNormalStyleFeatures = defaultStyle;
    documentObjects.insert("/OriginalNormalStyleFeatures", OriginalNormalStyleFeatures);
    QVariantList TextObjects;
    documentObjects.insert("/TextObjects", TextObjects);

    doc.insert("/DocumentResources", documentResources);
    doc.insert("/DocumentObjects", documentObjects);
    return doc;
}

QVariantHash defaultGrid {
    {"/GridIsOn", false},
    {"/ShowGrid", false},
    {"/GridSize", 18.0},
    {"/GridLeading", 22.0},
    {"/GridColor", QVariantHash{{"/Type", 1}, {"/Values", QVariantList({0, 0, 0, 1})}}},
    {"/GridLeadingFillColor", QVariantHash{{"/Type", 1}, {"/Values", QVariantList({0, 0, 0, 1})}}},
    {"/AlignLineHeightToGridFlags", false},
};

static QStringList simpleStyleAllowed {
    "/Font",
    "/FontSize",
    "/FauxBold",
    "/FauxItalic",
    "/AutoLeading",

    "/Leading",
    "/HorizontalScale",
    "/VerticalScale",
    "/Tracking",
    "/BaselineShift",

    "/FontCaps",
    "/FontBaseline",
    "/Ligatures",
    "/BaselineDirection",
    "/Tsume",

    "/StyleRunAlignment",
    "/Language",
    "/NoBreak",
    "/FillFlag",
    "/StrokeFlag",

    "/FillFirst",
    "/Kashida",
};
static QVariantHash simplifyStyleSheet(const QVariantHash complex) {
    QVariantHash simple;
    Q_FOREACH(const QString key, complex.keys()) {
        if (simpleStyleAllowed.contains(key)) {
            simple.insert(key, complex.value(key));
        }else if (key == "/StrokeColor" || key == "/FillColor") {
            simple.insert(key, complex.value(key).toHash().value("/Color"));
        } else if (key == "/UnderlinePosition") {
            bool val = complex.value(key).toBool();
            simple.insert("/Underline", val);
            simple.insert("/YUnderline", complex.value(key));
        } else if (key == "/StrikethroughPosition") {
            bool val = complex.value(key).toBool();
            simple.insert("/Strikethrough", val);
        } else if (key == "/AutoKerning") {
            bool val = complex.value(key).toBool();
            simple.insert("/AutoKern", val);
        } else if (key == "/LineWidth") {
            simple.insert("/OutlineWidth", complex.value(key));
        } else if (key == "/DiscretionaryLigatures") {
            simple.insert("/DLigatures", complex.value(key));
        }
    }
    simple.insert("/Kerning", 0.0);
    simple.insert("/HindiNumbers", false);
    simple.insert("/DiacriticPos", 2);
    return simple;
}
static QStringList simpleParagraphAllowed {
    "/Justification",
    "/FirstLineIndent",
    "/StartIndent",
    "/EndIndent",
    "/SpaceBefore",

    "/AutoHyphenate",
    "/HyphenatedWordSize",
    "/PreHyphen",
    "/PostHyphen",
    "/Zone",

    "/WordSpacing",
    "/LetterSpacing",
    "/GlyphSpacing",
    "/SpaceAfter",
    "/AutoLeading",

    "/LeadingType",
    "/Hanging",
    "/Burasagari",
    "/KinsokuOrder",
    "/EveryLineComposer",
};
static QVariantHash simplifyParagraphSheet(const QVariantHash complex) {
    QVariantHash simple;
    Q_FOREACH(const QString key, complex.keys()) {
        if (simpleParagraphAllowed.contains(key)) {
            simple.insert(key, complex.value(key));
        }
    }
    return simple;
}

QVariantHash KisTxt2Utils::tyShFromTxt2(const QVariantHash Txt2, const QRectF boundsInPx, int textIndex)
{
    QVariantHash tySh;

    const QVariantHash documentObjects = Txt2.value("/DocumentObjects").toHash();
    const QVariantList textObjects = documentObjects.value("/TextObjects").toList();
    QVariantHash textObject;
    if (textIndex < textObjects.size()) {
        textObject = textObjects.value(textIndex).toHash();
    }


    const QVariantHash model = textObject.value("/Model").toHash();
    const QVariantHash view = textObject.value("/View").toHash();

    const QVariantHash documentResources = Txt2.value("/DocumentResources").toHash();
    const QVariantList textFrames = documentResources.value("/TextFrameSet").toHash().value("/Resources").toList();

    QVariantHash engineDict;

    QVariantHash editor;
    editor.insert("/Text", model.value("/Text"));
    engineDict.insert("/Editor", editor);


    engineDict.insert("/GridInfo", defaultGrid);

    // paragraph
    const QVariantHash para = model.value("/ParagraphRun").toHash();
    const QVariantList paraRunArray = para.value("/RunArray").toList();
    // if we want to go over each entry, here's the moment to do so.
    const int length = paraRunArray.value(0).toHash().value("/Length").toInt();
    const QVariantHash paraSheet = paraRunArray.value(0).toHash().value("/RunData").toHash()["/ParagraphSheet"].toHash();
    const QVariantHash paragraphStyle = simplifyParagraphSheet(paraSheet.value("/Features").toHash());
    QVariantHash paragraphRun;
    paragraphRun["/RunLengthArray"] = QVariantList({QVariant(length)});
    QVariantHash paragraphAdjustments  = QVariantHash {
    {"/Axis", QVariantList({1, 0, 1})},
    {"/XY", QVariantList({0, 0})}
    };
    paragraphRun["/RunArray"] = QVariantList({ QVariantHash{
                                                {"/ParagraphSheet", paragraphStyle},
                                                {"/Adjustments", paragraphAdjustments}
                                            }
                                            });
    paragraphRun.insert("/IsJoinable", 1); //no idea what this means.
    paragraphRun.insert("/DefaultRunData", QVariantHash{
    {"/ParagraphSheet",
            QVariantHash{{"/DefaultStyleSheet", 0},
                        {"/Properties", QVariantHash()}} },
    {"/Adjustments", paragraphAdjustments}});

    engineDict.insert("/ParagraphRun", paragraphRun);

    const QVariantHash txt2StyleRun = model.value("/StyleRun").toHash();
    const QVariantList txt2RunArray = txt2StyleRun.value("/RunArray").toList();
    QVariantHash styleRun;

    QVariantList properStyleRun;
    QVariantList styleRunArray;
    Q_FOREACH(const QVariant entry, txt2RunArray) {
        QVariantHash properStyle;
        const QVariantHash fea = entry.toHash().value("/RunData").toHash().value("/StyleSheet").toHash().value("/Features").toHash();
        properStyle.insert("/StyleSheetData", simplifyStyleSheet(fea));
        QVariantHash s;
        s.insert("/StyleSheet", properStyle);
        properStyleRun.append(s);
        styleRunArray.append(entry.toHash().value("/Length").toInt());
    }
    styleRun.insert("/RunArray", properStyleRun);
    styleRun.insert("/RunLengthArray", styleRunArray);
    styleRun.insert("/IsJoinable", 2);

    styleRun.insert("/DefaultRunData", QVariantHash{{"/StyleSheet", QVariantHash{{"/StyleSheetData", QVariantHash()}} }});
    engineDict.insert("/StyleRun", styleRun);
    // rendered data...

    const int frameIndex = view.value("/Frames").toList().value(0).toHash().value("/Resource").toInt();
    const QVariantHash textFrame = textFrames.value(frameIndex).toHash().value("/Resource").toHash();
    const QVariantHash textFrameData = textFrame.value("/Data").toHash();
    QVariantHash rendered;
    int shapeType = textFrameData.value("/Type", QVariant(0)).toInt(); // 0 point, 1 paragraph, 2, text on path
    int writingDirection = textFrameData.value("/LineOrientation", QVariant(0)).toInt();
    QVariantHash photoshop = QVariantHash {{"/ShapeType", shapeType},
    {"/TransformPoint0", QVariantList({1.0, 0.0})},
    {"/TransformPoint1", QVariantList({0.0, 1.0})},
    {"/TransformPoint2", QVariantList({0.0, 0.0})}};
    if (shapeType == 0) {
        photoshop["/PointBase"] = QVariantList({0.0, 0.0});
    } else if (shapeType == 1) {
        // this is the bounding box of the paragraph shape.
        photoshop["/BoxBounds"] = QVariantList({0.0, 0.0, boundsInPx.width(), boundsInPx.height()});
    }
    QVariantHash renderChild = QVariantHash{
    {"/ShapeType", shapeType},
    {"/Procession", 0},
    {"/Lines", QVariantHash{{"/WritingDirection", writingDirection}, {"/Children", QVariantList()}}},
    {"/Cookie", QVariantHash{{"/Photoshop", photoshop}}}};
    rendered["/Version"] = 1;
    rendered["/Shapes"] = QVariantHash{{"/WritingDirection", writingDirection}, {"/Children", QVariantList({renderChild})}};

    engineDict.insert("/Rendered", rendered);

    // storysheet
    const QVariantHash storySheet = textObject.value("/StorySheet").toHash();
    engineDict.insert("/UseFractionalGlyphWidths", storySheet.value("/UseFractionalGlyphWidths", QVariant(true)).toBool());
    engineDict.insert("/AntiAlias", storySheet.value("/AntiAlias", QVariant(1)).toInt());

    QVariantHash resourceDict;

    const QVariantList docFontSet = documentResources.value("/FontSet").toHash().value("/Resources").toList();
    QVariantList fontSet;
    Q_FOREACH(QVariant entry, docFontSet) {
        const QVariantHash docFont = entry.toHash().value("/Resource").toHash();
        QVariantHash font = docFont.value("/Identifier").toHash();
        font.insert("/FontType", font.value("/Type"));
        font.remove("/Version");
        font.remove("/Type");
        font.insert("/Script", 0);
        font.insert("/Synthetic", 0);
        fontSet.append(font);
    }
    resourceDict.insert("/FontSet", fontSet);

    const QVariantHash documentSettings = documentObjects.value("/DocumentSettings").toHash();

    QVariantHash kinHard = kinsokuHard;
    kinHard.remove("/PredefinedTag");
    kinHard.insert("/Name", "PhotoshopKinsokuHard");
    QVariantHash kinSoft = kinsokuSoft;
    kinSoft.remove("/PredefinedTag");
    kinSoft.insert("/Name", "PhotoshopKinsokuSoft");
    resourceDict.insert("/KinsokuSet", QVariantList({kinHard, kinSoft}));
    resourceDict.insert("/MojiKumiSet", QVariantList( {QVariantHash{{"/InternalName", "Photoshop6MojiKumiSet1"}},
                                               QVariantHash{{"/InternalName", "Photoshop6MojiKumiSet2"}},
                                               QVariantHash{{"/InternalName", "Photoshop6MojiKumiSet3"}},
                                               QVariantHash{{"/InternalName", "Photoshop6MojiKumiSet4"}}
                                              }));
    resourceDict.insert("/SubscriptPosition", documentSettings.value("/SubscriptPosition"));
    resourceDict.insert("/SubscriptSize", documentSettings.value("/SubscriptSize"));
    resourceDict.insert("/SuperscriptPosition", documentSettings.value("/SuperscriptPosition"));
    resourceDict.insert("/SuperscriptSize", documentSettings.value("/SuperscriptSize"));
    resourceDict.insert("/SmallCapSize", documentSettings.value("/SmallCapSize"));
    resourceDict.insert("/TheNormalParagraphSheet", documentSettings.value("/NormalParagraphSheet"));
    resourceDict.insert("/TheNormalStyleSheet", documentSettings.value("/NormalStyleSheet"));

    const QVariantList docStyleSheetSets = documentResources.value("/StyleSheetSet").toHash().value("/Resources").toList();
    QVariantList resourceStyleSheetList;
    Q_FOREACH(QVariant entry, docStyleSheetSets) {
        const QVariantHash styleSheet = entry.toHash().value("/Resource").toHash();
        QVariantHash newSheet;
        newSheet.insert("/Name", styleSheet.value("/Name"));
        newSheet.insert("/StyleSheetData", simplifyStyleSheet(styleSheet.value("/Features").toHash()));
        resourceStyleSheetList.append(newSheet);
    }

    const QVariantList docParagraphSheetSets = documentResources.value("/ParagraphSheetSet").toHash().value("/Resources").toList();
    QVariantList resourceParagraphSheetList;
    Q_FOREACH(QVariant entry, docParagraphSheetSets) {
        const QVariantHash styleSheet = entry.toHash().value("/Resource").toHash();
        QVariantHash newSheet;
        newSheet.insert("/Name", styleSheet.value("/Name"));
        newSheet.insert("/Properties", simplifyParagraphSheet(styleSheet.value("/Features").toHash()));
        newSheet.insert("/DefaultStyleSheet", 0);
        resourceParagraphSheetList.append(newSheet);
    }

    resourceDict.insert("/StyleSheetSet", resourceStyleSheetList);
    resourceDict.insert("/ParagraphSheetSet", resourceParagraphSheetList);

    tySh.insert("/EngineDict", engineDict);
    tySh.insert("/DocumentResources", resourceDict);
    tySh.insert("/ResourceDict", resourceDict);
    return tySh;
}
