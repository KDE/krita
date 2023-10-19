/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_txt2_utls.h"
#include <QVariantHash>
#include <QVariant>
#include <QVariantList>

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
        {"/59", "/FillFirst"},

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
