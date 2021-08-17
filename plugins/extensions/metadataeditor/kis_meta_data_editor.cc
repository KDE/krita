/*
 *  SPDX-FileCopyrightText: 2007, 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_meta_data_editor.h"

#include <QDomDocument>
#include <QFile>
#include <QHeaderView>
#include <QTableView>

#include <KoResourcePaths.h>
#include <kis_debug.h>
#include <kis_icon.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_schema.h>
#include <kis_meta_data_schema_registry.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_value.h>
#include <klocalizedstring.h>

#include "kis_entry_editor.h"
#include "kis_meta_data_model.h"


struct KisMetaDataEditor::Private {
    KisMetaData::Store *store;
    KisMetaData::Store *temporaryStorage;
    QList<KisEntryEditor *> entryHandlers;
};

#define GET_VALUE(entryName, wdg, wdgPropertyName, editorSignal) \
    QString key = schema->generateQualifiedName(#entryName); \
    KisEntryEditor *ee = new KisEntryEditor(wdg, d->temporaryStorage, key, #wdgPropertyName, QString(), 0); \
    connect(wdg, editorSignal, ee, &KisEntryEditor::valueEdited); \
    d->entryHandlers.push_back(ee);

#define GET_ARRAY_VALUE(entryName, arrayIndex, wdg, wdgPropertyName, editorSignal) \
    QString key = schema->generateQualifiedName(#entryName); \
    KisEntryEditor *ee = new KisEntryEditor(wdg, d->temporaryStorage, key, #wdgPropertyName, QString(), arrayIndex); \
    connect(wdg, editorSignal, ee, &KisEntryEditor::valueEdited); \
    d->entryHandlers.push_back(ee);

#define GET_STRUCTURE_VALUE(entryName, structureField, wdg, wdgPropertyName, editorSignal) \
    QString key = schema->generateQualifiedName(#entryName); \
    KisEntryEditor *ee = new KisEntryEditor(wdg, d->temporaryStorage, key, #wdgPropertyName, #structureField, 0); \
    connect(wdg, editorSignal, ee, &KisEntryEditor::valueEdited); \
    d->entryHandlers.push_back(ee);

#define GET_DC_VALUE(entryName, wdgPropertyName, editorSignal) { \
    GET_VALUE(entryName, wdg->entryName, wdgPropertyName, editorSignal) \
}

#define GET_EXIF_VALUE(entryName, wdgPropertyName, editorSignal) { \
    GET_VALUE(entryName, wdg->edit##entryName, wdgPropertyName, editorSignal) \
}

#define GET_EXIF_ARRAY_VALUE(entryName, arrayIndex, wdgPropertyName, editorSignal) { \
    GET_ARRAY_VALUE(entryName, arrayIndex, wdg->edit##entryName, wdgPropertyName, editorSignal) \
}

#define GET_EXIF_STRUCTURE_VALUE(entryName, structureField, wdgPropertyName, editorSignal) { \
    GET_STRUCTURE_VALUE(entryName, structureField, wdg->edit##entryName##structureField, wdgPropertyName, editorSignal) \
}

KisMetaDataEditor::KisMetaDataEditor(QWidget *parent, KisMetaData::Store *store)
    : KPageDialog(parent)
    , d(new Private)
{
    d->temporaryStorage = new KisMetaData::Store(*store);
    d->store = store;

    // Add the Dublin Core widget
    {
        WdgDublinCore *wdg = new WdgDublinCore(this);

        {
            // copy from dublin core
            const KisMetaData::Schema *schema =
                KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);

            Q_ASSERT(schema);

            GET_DC_VALUE(creator, text, &QLineEdit::textEdited);
            GET_DC_VALUE(publisher, text, &QLineEdit::textEdited);
            GET_DC_VALUE(rights, text, &QLineEdit::textEdited);
            GET_DC_VALUE(date, date, &QDateTimeEdit::editingFinished);
            GET_DC_VALUE(title, text, &QLineEdit::textEdited);
            GET_DC_VALUE(description, plainText, &QTextEdit::textChanged);
        }

        KPageWidgetItem *page = new KPageWidgetItem(wdg, i18n("Dublin Core"));
        page->setIcon(KisIconUtils::loadIcon("user-identity"));
        addPage(page);
    }

    // Add the Exif widget
    {
        WdgExif *wdg = new WdgExif(this);

        {
            // copy from exif
            const KisMetaData::Schema *schema =
                KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::EXIFSchemaUri);

            Q_ASSERT(schema);

            /* exposure tab */
            GET_EXIF_VALUE(BrightnessValue, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(ExposureTime, text, &QLineEdit::textEdited);
            GET_EXIF_ARRAY_VALUE(ISOSpeedRatings, 0, value, QOverload<int>::of(&KisIntParseSpinBox::valueChanged));
            GET_EXIF_VALUE(ExposureMode, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(ExposureProgram, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(ExposureIndex, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(ExposureBiasValue, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(ApertureValue, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(ShutterSpeedValue, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(FNumber, text, &QLineEdit::textEdited);
            /*  lens tab  */
            GET_EXIF_VALUE(FocalLengthIn35mmFilm, value, QOverload<int>::of(&QSpinBox::valueChanged));
            GET_EXIF_VALUE(FocalLength, value, QOverload<int>::of(&QSpinBox::valueChanged));
            GET_EXIF_VALUE(MaxApertureValue, text, &QLineEdit::textEdited);
            /*  autofocus tab  */
            GET_EXIF_VALUE(MeteringMode, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(SubjectDistance, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(SubjectDistanceRange, currentIndex, QOverload<int>::of(&QComboBox::activated));
            /*  flash  */
            GET_EXIF_STRUCTURE_VALUE(Flash, Fired, isChecked, QOverload<bool>::of(&QCheckBox::toggled));
            GET_EXIF_STRUCTURE_VALUE(Flash, StrobeReturn, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_STRUCTURE_VALUE(Flash, Mode, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_STRUCTURE_VALUE(Flash, Function, isChecked, QOverload<bool>::of(&QCheckBox::toggled));
            GET_EXIF_STRUCTURE_VALUE(Flash, RedEyeMode, isChecked, QOverload<bool>::of(&QCheckBox::toggled));
            GET_EXIF_VALUE(FlashEnergy, text, &QLineEdit::textEdited);
            /*  postprocessing tab */
            GET_EXIF_VALUE(GainControl, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(LightSource, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(Sharpness, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(Contrast, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(WhiteBalance, currentIndex, QOverload<int>::of(&QComboBox::activated));
            /*  misc tab */
            GET_EXIF_VALUE(SceneCaptureType, currentIndex, QOverload<int>::of(&QComboBox::activated));
            GET_EXIF_VALUE(SensingMethod, currentIndex, QOverload<int>::of(&QComboBox::activated));
        }

        {
            // copy from exif (tiff schema)
            const KisMetaData::Schema *schema =
                KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::TIFFSchemaUri);

            Q_ASSERT(schema);

            /*  misc  */
            GET_EXIF_VALUE(Make, text, &QLineEdit::textEdited);
            GET_EXIF_VALUE(Model, text, &QLineEdit::textEdited);
        }

        KPageWidgetItem *page = new KPageWidgetItem(wdg, i18n("Exif"));
        page->setIcon(KisIconUtils::loadIcon("camera-photo"));
        addPage(page);
    }

    {
        // Add the list page
        QTableView *tableView = new QTableView;
        KisMetaDataModel *model = new KisMetaDataModel(d->store);
        tableView->setModel(model);
        tableView->verticalHeader()->setVisible(false);
        tableView->resizeColumnsToContents();
        KPageWidgetItem *page = new KPageWidgetItem(tableView, i18n("List"));
        page->setIcon(KisIconUtils::loadIcon("format-list-unordered"));
        addPage(page);
    }
}

KisMetaDataEditor::~KisMetaDataEditor()
{
    Q_FOREACH (KisEntryEditor *e, d->entryHandlers) {
        delete e;
    }
    delete d->temporaryStorage;
    delete d;
}

void KisMetaDataEditor::accept()
{
    KPageDialog::accept();

    d->store->copyFrom(d->temporaryStorage);
}
