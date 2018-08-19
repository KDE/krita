/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SvgTextEditor.h"

#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QSvgGenerator>
#include <QTabWidget>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidgetAction>

#include <kcharselect.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <kguiitem.h>

#include <KoDialog.h>
#include <KoResourcePaths.h>
#include <KoSvgTextShape.h>
#include <KoSvgTextShapeMarkupConverter.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorPopupAction.h>
#include <svg/SvgUtil.h>

#include <kis_icon.h>
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <kis_action_registry.h>

#include "kis_font_family_combo_box.h"
#include "FontSizeAction.h"
#include "kis_signals_blocker.h"

SvgTextEditor::SvgTextEditor(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags)
    , m_page(new QWidget(this))
#ifndef Q_OS_WIN
    , m_charSelectDialog(new KoDialog(this))
#endif
{
    m_textEditorWidget.setupUi(m_page);
    setCentralWidget(m_page);

    m_textEditorWidget.chkVertical->setVisible(false);
#ifndef Q_OS_WIN
    KCharSelect *charSelector = new KCharSelect(m_charSelectDialog, 0, KCharSelect::AllGuiElements);
    m_charSelectDialog->setMainWidget(charSelector);
    connect(charSelector, SIGNAL(currentCharChanged(QChar)), SLOT(insertCharacter(QChar)));
    m_charSelectDialog->hide();
    m_charSelectDialog->setButtons(KoDialog::Close);
#endif
    connect(m_textEditorWidget.buttons, SIGNAL(accepted()), this, SLOT(save()));
    connect(m_textEditorWidget.buttons, SIGNAL(rejected()), this, SLOT(close()));
    connect(m_textEditorWidget.buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogButtonClicked(QAbstractButton*)));

    KConfigGroup cg(KSharedConfig::openConfig(), "SvgTextTool");
    actionCollection()->setConfigGroup("SvgTextTool");
    actionCollection()->setComponentName("svgtexttool");
    actionCollection()->setComponentDisplayName(i18n("Text Tool"));

    QByteArray state;
    if (cg.hasKey("WindowState")) {
        state = cg.readEntry("State", state);
        state = QByteArray::fromBase64(state);
        // One day will need to load the version number, but for now, assume 0
        restoreState(state);
    }

    setAcceptDrops(true);
    //setStandardToolBarMenuEnabled(true);
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    m_syntaxHighlighter = new BasicXMLSyntaxHighlighter(m_textEditorWidget.svgTextEdit);
    m_textEditorWidget.svgTextEdit->setFont(QFontDatabase().systemFont(QFontDatabase::FixedFont));

    createActions();
    // If we have customized the toolbars, load that first
    setLocalXMLFile(KoResourcePaths::locateLocal("data", "svgtexttool.xmlgui"));
    setXMLFile(":/kxmlgui5/svgtexttool.xmlgui");

    guiFactory()->addClient(this);

    // Create and plug toolbar list for Settings menu
    QList<QAction *> toolbarList;
    Q_FOREACH (QWidget* it, guiFactory()->containers("ToolBar")) {
        KToolBar * toolBar = ::qobject_cast<KToolBar *>(it);
        if (toolBar) {
            toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
            KToggleAction* act = new KToggleAction(i18n("Show %1 Toolbar", toolBar->windowTitle()), this);
            actionCollection()->addAction(toolBar->objectName().toUtf8(), act);
            act->setCheckedState(KGuiItem(i18n("Hide %1 Toolbar", toolBar->windowTitle())));
            connect(act, SIGNAL(toggled(bool)), this, SLOT(slotToolbarToggled(bool)));
            act->setChecked(!toolBar->isHidden());
            toolbarList.append(act);
        }
    }
    plugActionList("toolbarlist", toolbarList);
    connect(m_textEditorWidget.textTab, SIGNAL(currentChanged(int)), this, SLOT(switchTextEditorTab()));
    switchTextEditorTab();

    m_textEditorWidget.richTextEdit->document()->setDefaultStyleSheet("p {margin:0px;}");

    applySettings();

}

SvgTextEditor::~SvgTextEditor()
{
    KConfigGroup g(KSharedConfig::openConfig(), "SvgTextTool");
    QByteArray ba = saveState();
    g.writeEntry("windowState", ba.toBase64());
}


void SvgTextEditor::setShape(KoSvgTextShape *shape)
{
    m_shape = shape;
    if (m_shape) {
        KoSvgTextShapeMarkupConverter converter(m_shape);

        QString svg;
        QString styles;
        QTextDocument *doc = m_textEditorWidget.richTextEdit->document();

        if (converter.convertToSvg(&svg, &styles)) {
            m_textEditorWidget.svgTextEdit->setPlainText(svg);
            m_textEditorWidget.svgStylesEdit->setPlainText(styles);
            m_textEditorWidget.svgTextEdit->document()->setModified(false);
            if (converter.convertSvgToDocument(svg, doc)) {
                m_textEditorWidget.richTextEdit->setDocument(doc);
            }
        }
        else {
            QMessageBox::warning(this, i18n("Conversion failed"), "Could not get svg text from the shape:\n" + converter.errors().join('\n') + "\n" + converter.warnings().join('\n'));
        }
    }

}

void SvgTextEditor::save()
{
    if (m_shape) {
        if (m_textEditorWidget.textTab->currentIndex() == Richtext) {

            QString svg;
            QString styles = m_textEditorWidget.svgStylesEdit->document()->toPlainText();
            KoSvgTextShapeMarkupConverter converter(m_shape);

            if (!converter.convertDocumentToSvg(m_textEditorWidget.richTextEdit->document(), &svg)) {
                    qWarning()<<"new converter doesn't work!";
            }
            m_textEditorWidget.richTextEdit->document()->setModified(false);
            emit textUpdated(m_shape, svg, styles);
        }
        else {
            emit textUpdated(m_shape, m_textEditorWidget.svgTextEdit->document()->toPlainText(), m_textEditorWidget.svgStylesEdit->document()->toPlainText());
            m_textEditorWidget.svgTextEdit->document()->setModified(false);
        }
    }

}

void SvgTextEditor::switchTextEditorTab()
{
    KoSvgTextShape shape;
    KoSvgTextShapeMarkupConverter converter(&shape);

    if (m_currentEditor) {
        disconnect(m_currentEditor->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setModified(bool)));
    }

    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        //first, make buttons checkable
        enableRichTextActions(true);

        //then connect the cursor change to the checkformat();
        connect(m_textEditorWidget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));


        if (m_shape) {
            QTextDocument *doc = m_textEditorWidget.richTextEdit->document();
            if (!converter.convertSvgToDocument(m_textEditorWidget.svgTextEdit->document()->toPlainText(), doc)) {
                qWarning()<<"new converter svgToDoc doesn't work!";
            }
            m_textEditorWidget.richTextEdit->setDocument(doc);
        }
        m_currentEditor = m_textEditorWidget.richTextEdit;
    }
    else {
        //first, make buttons uncheckable
        enableRichTextActions(false);
        disconnect(m_textEditorWidget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));

        // Convert the rich text to svg and styles strings
        if (m_shape) {
            QString svg;
            QString styles;

            if (!converter.convertDocumentToSvg(m_textEditorWidget.richTextEdit->document(), &svg)) {
                    qWarning()<<"new converter docToSVG doesn't work!";
            }
            m_textEditorWidget.svgTextEdit->setPlainText(svg);
        }
        m_currentEditor = m_textEditorWidget.svgTextEdit;
    }

    connect(m_currentEditor->document(), SIGNAL(modificationChanged(bool)), SLOT(setModified(bool)));
}

void SvgTextEditor::checkFormat()
{
    QTextCharFormat format = m_textEditorWidget.richTextEdit->textCursor().charFormat();
    QTextBlockFormat blockFormat = m_textEditorWidget.richTextEdit->textCursor().blockFormat();
    if (format.fontWeight() > QFont::Normal) {
        actionCollection()->action("svg_weight_bold")->setChecked(true);
    } else {
        actionCollection()->action("svg_weight_bold")->setChecked(false);
    }
    actionCollection()->action("svg_format_italic")->setChecked(format.fontItalic());
    actionCollection()->action("svg_format_underline")->setChecked(format.fontUnderline());
    actionCollection()->action("svg_format_strike_through")->setChecked(format.fontStrikeOut());

    FontSizeAction *fontSizeAction = qobject_cast<FontSizeAction*>(actionCollection()->action("svg_font_size"));
    fontSizeAction->setFontSize(format.font().pointSize());

    KoColor fg(format.foreground().color(), KoColorSpaceRegistry::instance()->rgb8());
    qobject_cast<KoColorPopupAction*>(actionCollection()->action("svg_format_textcolor"))->setCurrentColor(fg);

    KoColor bg(format.foreground().color(), KoColorSpaceRegistry::instance()->rgb8());
    qobject_cast<KoColorPopupAction*>(actionCollection()->action("svg_background_color"))->setCurrentColor(bg);



    KisFontComboBoxes* fontComboBox = qobject_cast<KisFontComboBoxes*>(qobject_cast<QWidgetAction*>(actionCollection()->action("svg_font"))->defaultWidget());

    KisSignalsBlocker b(fontComboBox); // this prevents setting the entire selection to one font
    fontComboBox->setCurrentFont(format.font());

    QDoubleSpinBox *spnLineHeight = qobject_cast<QDoubleSpinBox*>(qobject_cast<QWidgetAction*>(actionCollection()->action("svg_line_height"))->defaultWidget());
    if (blockFormat.lineHeightType()==QTextBlockFormat::SingleHeight) {
        spnLineHeight->setValue(100.0);
    } else if(blockFormat.lineHeightType()==QTextBlockFormat::ProportionalHeight) {
        spnLineHeight->setValue(double(blockFormat.lineHeight()));
    }
}

void SvgTextEditor::undo()
{
    m_currentEditor->undo();
}

void SvgTextEditor::redo()
{
    m_currentEditor->redo();
}

void SvgTextEditor::cut()
{
    m_currentEditor->cut();
}

void SvgTextEditor::copy()
{
    m_currentEditor->copy();
}

void SvgTextEditor::paste()
{
    m_currentEditor->paste();
}

void SvgTextEditor::selectAll()
{
    m_currentEditor->selectAll();
}

void SvgTextEditor::deselect()
{
    QTextCursor cursor(m_currentEditor->textCursor());
    cursor.clearSelection();
    m_currentEditor->setTextCursor(cursor);
}

void SvgTextEditor::find()
{
    QDialog *findDialog = new QDialog(this);
    findDialog->setWindowTitle(i18n("Find Text"));
    QFormLayout *layout = new QFormLayout();
    findDialog->setLayout(layout);
    QLineEdit *lnSearchKey = new QLineEdit();
    layout->addRow(i18n("Find:"), lnSearchKey);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    findDialog->layout()->addWidget(buttons);
    connect(buttons, SIGNAL(accepted()), findDialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), findDialog, SLOT(reject()));


    if (findDialog->exec()==QDialog::Accepted) {
        m_searchKey = lnSearchKey->text();
        m_currentEditor->find(m_searchKey);
    }
}

void SvgTextEditor::findNext()
{
    if (!m_currentEditor->find(m_searchKey)) {
        QTextCursor cursor(m_currentEditor->textCursor());
        cursor.movePosition(QTextCursor::Start);
        m_currentEditor->setTextCursor(cursor);
        m_currentEditor->find(m_searchKey);
    }
}

void SvgTextEditor::findPrev()
{
    if (!m_currentEditor->find(m_searchKey,QTextDocument::FindBackward)) {
        QTextCursor cursor(m_currentEditor->textCursor());
        cursor.movePosition(QTextCursor::End);
        m_currentEditor->setTextCursor(cursor);
        m_currentEditor->find(m_searchKey,QTextDocument::FindBackward);
    }
}

void SvgTextEditor::replace()
{
    QDialog *findDialog = new QDialog(this);
    findDialog->setWindowTitle(i18n("Find and Replace all"));
    QFormLayout *layout = new QFormLayout();
    findDialog->setLayout(layout);
    QLineEdit *lnSearchKey = new QLineEdit();
    QLineEdit *lnReplaceKey = new QLineEdit();
    layout->addRow(i18n("Find:"), lnSearchKey);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    layout->addRow(i18n("Replace:"), lnReplaceKey);
    findDialog->layout()->addWidget(buttons);
    connect(buttons, SIGNAL(accepted()), findDialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), findDialog, SLOT(reject()));


    if (findDialog->exec()==QDialog::Accepted) {
        QString search = lnSearchKey->text();
        QString replace = lnReplaceKey->text();
        QTextCursor cursor(m_currentEditor->textCursor());
        cursor.movePosition(QTextCursor::Start);
        m_currentEditor->setTextCursor(cursor);
        while(m_currentEditor->find(search)) {
            m_currentEditor->textCursor().removeSelectedText();
            m_currentEditor->textCursor().insertText(replace);
        }

    }
}


void SvgTextEditor::zoomOut()
{
    m_currentEditor->zoomOut();
}

void SvgTextEditor::zoomIn()
{
    m_currentEditor->zoomIn();
}

#ifndef Q_OS_WIN
void SvgTextEditor::showInsertSpecialCharacterDialog()
{
    m_charSelectDialog->setVisible(!m_charSelectDialog->isVisible());
}

void SvgTextEditor::insertCharacter(const QChar &c)
{
    m_currentEditor->textCursor().insertText(QString(c));
}
#endif

void SvgTextEditor::setTextBold(QFont::Weight weight)
{
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight() > QFont::Normal && weight==QFont::Bold) {
            format.setFontWeight(QFont::Normal);
        } else {
            format.setFontWeight(weight);
        }
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-weight:700;\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setTextWeightLight()
{
    if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight() < QFont::Normal) {
        setTextBold(QFont::Normal);
    } else {
        setTextBold(QFont::Light);
    }
}

void SvgTextEditor::setTextWeightNormal()
{
    setTextBold(QFont::Normal);
}

void SvgTextEditor::setTextWeightDemi()
{
    if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight() != QFont::Normal) {
        setTextBold(QFont::Normal);
    } else {
        setTextBold(QFont::DemiBold);
    }
}

void SvgTextEditor::setTextWeightBlack()
{
    if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal) {
        setTextBold(QFont::Normal);
    } else {
        setTextBold(QFont::Black);
    }
}

void SvgTextEditor::setTextItalic(QFont::Style style)
{
    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
    QString fontStyle = "inherit";

    if (style == QFont::StyleItalic) {
        fontStyle = "italic";
    } else if(style == QFont::StyleOblique) {
        fontStyle = "oblique";
    }

    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontItalic(!m_textEditorWidget.richTextEdit->textCursor().charFormat().fontItalic());
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    }
    else {
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-style:"+fontStyle+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setTextDecoration(KoSvgText::TextDecoration decor)
{
    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
    QTextCharFormat currentFormat = m_textEditorWidget.richTextEdit->textCursor().charFormat();
    QTextCharFormat format;
    QString textDecoration = "inherit";

    if (decor == KoSvgText::DecorationUnderline) {
        textDecoration = "underline";
        if (currentFormat.fontUnderline()) {
            format.setFontUnderline(false);
        }
        else {
            format.setFontUnderline(true);
        }
        format.setFontOverline(false);
        format.setFontStrikeOut(false);
    }
    else if (decor == KoSvgText::DecorationLineThrough) {
        textDecoration = "line-through";
        format.setFontUnderline(false);
        format.setFontOverline(false);
        if (currentFormat.fontStrikeOut()) {
            format.setFontStrikeOut(false);
        }
        else {
            format.setFontStrikeOut(true);
        }
    }
    else if (decor == KoSvgText::DecorationOverline) {
        textDecoration = "overline";
        format.setFontUnderline(false);
        if (currentFormat.fontOverline()) {
            format.setFontOverline(false);
        }
        else {
            format.setFontOverline(true);
        }
        format.setFontStrikeOut(false);
    }

    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    }
    else {
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"text-decoration:" + textDecoration + ";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setTextUnderline()
{
    setTextDecoration(KoSvgText::DecorationUnderline);
}

void SvgTextEditor::setTextOverline()
{
    setTextDecoration(KoSvgText::DecorationOverline);
}

void SvgTextEditor::setTextStrikethrough()
{
    setTextDecoration(KoSvgText::DecorationLineThrough);
}

void SvgTextEditor::setTextSubscript()
{
    QTextCharFormat format = m_textEditorWidget.richTextEdit->textCursor().charFormat();
    if (format.verticalAlignment()==QTextCharFormat::AlignSubScript) {
        format.setVerticalAlignment(QTextCharFormat::AlignNormal);
    } else {
        format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
    }
    m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
}

void SvgTextEditor::setTextSuperScript()
{
    QTextCharFormat format = m_textEditorWidget.richTextEdit->textCursor().charFormat();
    if (format.verticalAlignment()==QTextCharFormat::AlignSuperScript) {
        format.setVerticalAlignment(QTextCharFormat::AlignNormal);
    } else {
        format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    }
    m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
}

void SvgTextEditor::increaseTextSize()
{
    QTextCharFormat format;
    int pointSize = m_textEditorWidget.richTextEdit->textCursor().charFormat().font().pointSize();
    if (pointSize<0) {
        pointSize = m_textEditorWidget.richTextEdit->textCursor().charFormat().font().pixelSize();
    }
    format.setFontPointSize(pointSize+1.0);
    m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
}

void SvgTextEditor::decreaseTextSize()
{
    QTextCharFormat format;
    int pointSize = m_textEditorWidget.richTextEdit->textCursor().charFormat().font().pointSize();
    if (pointSize<1) {
        pointSize = m_textEditorWidget.richTextEdit->textCursor().charFormat().font().pixelSize();
    }
    format.setFontPointSize(qMax(pointSize-1.0, 1.0));
    m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
}

void SvgTextEditor::setLineHeight(double lineHeightPercentage)
{
    QTextBlockFormat format = m_textEditorWidget.richTextEdit->textCursor().blockFormat();
    format.setLineHeight(lineHeightPercentage, QTextBlockFormat::ProportionalHeight);
     m_textEditorWidget.richTextEdit->textCursor().mergeBlockFormat(format);
}


void SvgTextEditor::alignLeft()
{
    QTextBlockFormat format = m_textEditorWidget.richTextEdit->textCursor().blockFormat();
    format.setAlignment(Qt::AlignLeft);
    m_textEditorWidget.richTextEdit->textCursor().mergeBlockFormat(format);
}

void SvgTextEditor::alignRight()
{
    QTextBlockFormat format = m_textEditorWidget.richTextEdit->textCursor().blockFormat();
    format.setAlignment(Qt::AlignRight);
    m_textEditorWidget.richTextEdit->textCursor().mergeBlockFormat(format);
}

void SvgTextEditor::alignCenter()
{
    QTextBlockFormat format = m_textEditorWidget.richTextEdit->textCursor().blockFormat();
    format.setAlignment(Qt::AlignCenter);
    m_textEditorWidget.richTextEdit->textCursor().mergeBlockFormat(format);
}

void SvgTextEditor::alignJustified()
{
    QTextBlockFormat format = m_textEditorWidget.richTextEdit->textCursor().blockFormat();
    format.setAlignment(Qt::AlignJustify);
    m_textEditorWidget.richTextEdit->textCursor().mergeBlockFormat(format);
}

void SvgTextEditor::setSettings()
{
    KoDialog settingsDialog(this);
    Ui_WdgSvgTextSettings textSettings;
    QWidget *settingsPage = new QWidget(&settingsDialog, 0);
    settingsDialog.setMainWidget(settingsPage);
    textSettings.setupUi(settingsPage);

    // get the settings and initialize the dialog
    KConfigGroup cfg(KSharedConfig::openConfig(), "SvgTextTool");

    QStringList selectedWritingSystems = cfg.readEntry("selectedWritingSystems", "").split(",");

    QList<QFontDatabase::WritingSystem> scripts = QFontDatabase().writingSystems();
    QStandardItemModel *writingSystemsModel = new QStandardItemModel(&settingsDialog);
    for (int s = 0; s < scripts.size(); s ++) {
        QString writingSystem = QFontDatabase().writingSystemName(scripts.at(s));
        QStandardItem *script = new QStandardItem(writingSystem);
        script->setCheckable(true);
        script->setCheckState(selectedWritingSystems.contains(QString::number(scripts.at(s))) ? Qt::Checked : Qt::Unchecked);
        script->setData((int)scripts.at(s));
        writingSystemsModel->appendRow(script);
    }
    textSettings.lwScripts->setModel(writingSystemsModel);

    EditorMode mode = (EditorMode)cfg.readEntry("EditorMode", (int)Both);
    switch(mode) {
    case(RichText):
        textSettings.radioRichText->setChecked(true);
        break;
    case(SvgSource):
        textSettings.radioSvgSource->setChecked(true);
        break;
    case(Both):
        textSettings.radioBoth->setChecked(true);
    }

    QColor background = cfg.readEntry("colorEditorBackground", qApp->palette().background().color());
    textSettings.colorEditorBackground->setColor(background);
    textSettings.colorEditorForeground->setColor(cfg.readEntry("colorEditorForeground", qApp->palette().text().color()));

    textSettings.colorKeyword->setColor(cfg.readEntry("colorKeyword", QColor(background.value() < 100 ? Qt::cyan : Qt::blue)));
    textSettings.chkBoldKeyword->setChecked(cfg.readEntry("BoldKeyword", true));
    textSettings.chkItalicKeyword->setChecked(cfg.readEntry("ItalicKeyword", false));

    textSettings.colorElement->setColor(cfg.readEntry("colorElement", QColor(background.value() < 100 ? Qt::magenta : Qt::darkMagenta)));
    textSettings.chkBoldElement->setChecked(cfg.readEntry("BoldElement", true));
    textSettings.chkItalicElement->setChecked(cfg.readEntry("ItalicElement", false));

    textSettings.colorAttribute->setColor(cfg.readEntry("colorAttribute", QColor(background.value() < 100 ? Qt::green : Qt::darkGreen)));
    textSettings.chkBoldAttribute->setChecked(cfg.readEntry("BoldAttribute", true));
    textSettings.chkItalicAttribute->setChecked(cfg.readEntry("ItalicAttribute", true));

    textSettings.colorValue->setColor(cfg.readEntry("colorValue", QColor(background.value() < 100 ? Qt::red: Qt::darkRed)));
    textSettings.chkBoldValue->setChecked(cfg.readEntry("BoldValue", true));
    textSettings.chkItalicValue->setChecked(cfg.readEntry("ItalicValue", false));

    textSettings.colorComment->setColor(cfg.readEntry("colorComment", QColor(background.value() < 100 ? Qt::lightGray : Qt::gray)));
    textSettings.chkBoldComment->setChecked(cfg.readEntry("BoldComment", false));
    textSettings.chkItalicComment->setChecked(cfg.readEntry("ItalicComment", false));

    settingsDialog.setButtons(KoDialog::Ok | KoDialog::Cancel);
    if (settingsDialog.exec() == QDialog::Accepted) {
        // save  and set the settings
        QStringList writingSystems;
        for (int i = 0; i < writingSystemsModel->rowCount(); i++) {
            QStandardItem *item = writingSystemsModel->item(i);
            if (item->checkState() == Qt::Checked) {
                writingSystems.append(QString::number(item->data().toInt()));
            }
        }
        if (!writingSystems.isEmpty()) {
            cfg.writeEntry("selectedWritingSystems", writingSystems.join(','));
        }

        if (textSettings.radioRichText->isChecked()) {
            cfg.writeEntry("EditorMode", (int)Richtext);
        }
        else if (textSettings.radioSvgSource->isChecked()) {
            cfg.writeEntry("EditorMode", (int)SvgSource);
        }
        else  if (textSettings.radioBoth->isChecked()) {
            cfg.writeEntry("EditorMode", (int)Both);
        }

        cfg.writeEntry("colorEditorBackground", textSettings.colorEditorBackground->color());
        cfg.writeEntry("colorEditorForeground", textSettings.colorEditorForeground->color());

        cfg.writeEntry("colorKeyword", textSettings.colorKeyword->color());
        cfg.writeEntry("BoldKeyword", textSettings.chkBoldKeyword->isChecked());
        cfg.writeEntry("ItalicKeyWord", textSettings.chkItalicKeyword->isChecked());

        cfg.writeEntry("colorElement", textSettings.colorElement->color());
        cfg.writeEntry("BoldElement", textSettings.chkBoldElement->isChecked());
        cfg.writeEntry("ItalicElement", textSettings.chkItalicElement->isChecked());

        cfg.writeEntry("colorAttribute", textSettings.colorAttribute->color());
        cfg.writeEntry("BoldAttribute", textSettings.chkBoldAttribute->isChecked());
        cfg.writeEntry("ItalicAttribute", textSettings.chkItalicAttribute->isChecked());

        cfg.writeEntry("colorValue", textSettings.colorValue->color());
        cfg.writeEntry("BoldValue", textSettings.chkBoldValue->isChecked());
        cfg.writeEntry("ItalicValue", textSettings.chkItalicValue->isChecked());

        cfg.writeEntry("colorComment", textSettings.colorComment->color());
        cfg.writeEntry("BoldComment", textSettings.chkBoldComment->isChecked());
        cfg.writeEntry("ItalicComment", textSettings.chkItalicComment->isChecked());

        applySettings();
    }
}

void SvgTextEditor::slotToolbarToggled(bool)
{

}


void SvgTextEditor::setFontColor(const KoColor &c)
{
    QColor color = c.toQColor();
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setForeground(QBrush(color));
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    }
    else {
        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan fill=\""+color.name()+"\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setBackgroundColor(const KoColor &c)
{
    QColor color = c.toQColor();
    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan stroke=\""+color.name()+"\">" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::setModified(bool modified)
{
    if (modified) {
        m_textEditorWidget.buttons->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    }
    else {
        m_textEditorWidget.buttons->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Close);
    }
}

void SvgTextEditor::dialogButtonClicked(QAbstractButton *button)
{
    if (m_textEditorWidget.buttons->standardButton(button) == QDialogButtonBox::Discard) {
        if (QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("You have modified the text. Discard changes?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            close();
        }
    }
}

void SvgTextEditor::setFont(const QString &fontName)
{
    QFont font;
    font.fromString(fontName);
    QTextCharFormat curFormat = m_textEditorWidget.richTextEdit->textCursor().charFormat();
    font.setPointSize(curFormat.font().pointSize());

    QTextCharFormat format;
    //This disables the style being set from the font-comboboxes too, so we need to rethink how we use that.
    format.setFontFamily(font.family());
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-family:"+font.family()+" "+font.styleName()+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setFontSize(qreal fontSize)
{
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontPointSize(fontSize);
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-size:" + QString::number(fontSize) + ";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setBaseline(KoSvgText::BaselineShiftMode)
{

    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
    if (cursor.hasSelection()) {
        QString selectionModified = "<tspan style=\"font-size:50%;baseline-shift:super;\">" + cursor.selectedText() + "</tspan>";
        cursor.removeSelectedText();
        cursor.insertText(selectionModified);
    }
}

void SvgTextEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 7;
        m_textEditorWidget.svgTextEdit->zoomOut(numSteps);
        event->accept();
    }
}

void SvgTextEditor::applySettings()
{
    KConfigGroup cfg(KSharedConfig::openConfig(), "SvgTextTool");

    EditorMode mode = (EditorMode)cfg.readEntry("EditorMode", (int)Both);

    QWidget *richTab = m_textEditorWidget.richTab;
    QWidget *svgTab = m_textEditorWidget.svgTab;

    m_page->setUpdatesEnabled(false);
    m_textEditorWidget.textTab->clear();

    switch(mode) {
    case(RichText):
        m_textEditorWidget.textTab->addTab(richTab, i18n("Rich text"));
        break;
    case(SvgSource):
        m_textEditorWidget.textTab->addTab(svgTab, i18n("SVG Source"));
        break;
    case(Both):
        m_textEditorWidget.textTab->addTab(richTab, i18n("Rich text"));
        m_textEditorWidget.textTab->addTab(svgTab, i18n("SVG Source"));
    }

    m_syntaxHighlighter->setFormats();

    QPalette palette = m_textEditorWidget.svgTextEdit->palette();

    QColor background = cfg.readEntry("colorEditorBackground", qApp->palette().background().color());
    palette.setBrush(QPalette::Active, QPalette::Background, QBrush(background));
    QColor foreground = cfg.readEntry("colorEditorForeground", qApp->palette().text().color());
    palette.setBrush(QPalette::Active, QPalette::Text, QBrush(foreground));

    QStringList selectedWritingSystems = cfg.readEntry("selectedWritingSystems", "").split(",");

    QVector<QFontDatabase::WritingSystem> writingSystems;
    for (int i=0; i<selectedWritingSystems.size(); i++) {
        writingSystems.append((QFontDatabase::WritingSystem)QString(selectedWritingSystems.at(i)).toInt());
    }
    qobject_cast<KisFontComboBoxes*>(qobject_cast<QWidgetAction*>(actionCollection()->action("svg_font"))->defaultWidget())->refillComboBox(writingSystems);

    m_page->setUpdatesEnabled(true);
}

QAction *SvgTextEditor::createAction(const QString &name, const char *member)
{
    QAction *action = new QAction(this);
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();
    actionRegistry->propertizeAction(name, action);

    actionCollection()->addAction(name, action);
    QObject::connect(action, SIGNAL(triggered(bool)), this, member);
    return action;
}


void SvgTextEditor::createActions()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();


    // File: new, open, save, save as, close
    KStandardAction::save(this, SLOT(save()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());

    // Edit
    KStandardAction::undo(this, SLOT(undo()), actionCollection());
    KStandardAction::redo(this, SLOT(redo()), actionCollection());
    KStandardAction::cut(this, SLOT(cut()), actionCollection());
    KStandardAction::copy(this, SLOT(copy()), actionCollection());
    KStandardAction::paste(this, SLOT(paste()), actionCollection());
    KStandardAction::selectAll(this, SLOT(selectAll()), actionCollection());
    KStandardAction::deselect(this, SLOT(deselect()), actionCollection());
    KStandardAction::find(this, SLOT(find()), actionCollection());
    KStandardAction::findNext(this, SLOT(findNext()), actionCollection());
    KStandardAction::findPrev(this, SLOT(findPrev()), actionCollection());
    KStandardAction::replace(this, SLOT(replace()), actionCollection());

    // View
    KStandardAction::zoomOut(this, SLOT(zoomOut()), actionCollection());
    KStandardAction::zoomIn(this, SLOT(zoomIn()), actionCollection());
#ifndef Q_OS_WIN
    // Insert:
    QAction * insertAction = createAction("svg_insert_special_character",
                                          SLOT(showInsertSpecialCharacterDialog()));
    insertAction->setCheckable(true);
    insertAction->setChecked(false);
#endif
    // Format:
    m_richTextActions << createAction("svg_weight_bold",
                                      SLOT(setTextBold()));

    m_richTextActions << createAction("svg_format_italic",
                                      SLOT(setTextItalic()));

    m_richTextActions << createAction("svg_format_underline",
                                      SLOT(setTextUnderline()));

    m_richTextActions << createAction("svg_format_strike_through",
                                      SLOT(setTextStrikethrough()));

    m_richTextActions << createAction("svg_format_superscript",
                                      SLOT(setTextSuperScript()));

    m_richTextActions << createAction("svg_format_subscript",
                                      SLOT(setTextSubscript()));

    m_richTextActions << createAction("svg_weight_light",
                                      SLOT(setTextWeightLight()));

    m_richTextActions << createAction("svg_weight_normal",
                                      SLOT(setTextWeightNormal()));

    m_richTextActions << createAction("svg_weight_demi",
                                      SLOT(setTextWeightDemi()));

    m_richTextActions << createAction("svg_weight_black",
                                      SLOT(setTextWeightBlack()));

    m_richTextActions << createAction("svg_increase_font_size",
                                      SLOT(increaseTextSize()));

    m_richTextActions << createAction("svg_decrease_font_size",
                                      SLOT(decreaseTextSize()));

    m_richTextActions << createAction("svg_align_left",
                                      SLOT(alignLeft()));

    m_richTextActions << createAction("svg_align_right",
                                      SLOT(alignRight()));

    m_richTextActions << createAction("svg_align_center",
                                      SLOT(alignCenter()));

//    m_richTextActions << createAction("svg_align_justified",
//                                      SLOT(alignJustified()));

    // Settings
    m_richTextActions << createAction("svg_settings",
                                      SLOT(setSettings()));

    QWidgetAction *fontComboAction = new QWidgetAction(this);
    fontComboAction->setToolTip(i18n("Font"));
    KisFontComboBoxes *fontCombo = new KisFontComboBoxes();
    connect(fontCombo, SIGNAL(fontChanged(QString)), SLOT(setFont(QString)));
    fontComboAction->setDefaultWidget(fontCombo);
    actionCollection()->addAction("svg_font", fontComboAction);
    m_richTextActions << fontComboAction;
    actionRegistry->propertizeAction("svg_font", fontComboAction);

    QWidgetAction *fontSizeAction = new FontSizeAction(this);
    fontSizeAction->setToolTip(i18n("Size"));
    connect(fontSizeAction, SIGNAL(fontSizeChanged(qreal)), this, SLOT(setFontSize(qreal)));
    actionCollection()->addAction("svg_font_size", fontSizeAction);
    m_richTextActions << fontSizeAction;
    actionRegistry->propertizeAction("svg_font_size", fontSizeAction);

    KoColorPopupAction *fgColor = new KoColorPopupAction(this);
    fgColor->setCurrentColor(QColor(Qt::black));
    fgColor->setToolTip(i18n("Text Color"));
    connect(fgColor, SIGNAL(colorChanged(KoColor)), SLOT(setFontColor(KoColor)));
    actionCollection()->addAction("svg_format_textcolor", fgColor);
    m_richTextActions << fgColor;
    actionRegistry->propertizeAction("svg_format_textcolor", fgColor);

    KoColorPopupAction *bgColor = new KoColorPopupAction(this);
    bgColor->setCurrentColor(QColor(Qt::white));
    bgColor->setToolTip(i18n("Background Color"));
    connect(bgColor, SIGNAL(colorChanged(KoColor)), SLOT(setBackgroundColor(KoColor)));
    actionCollection()->addAction("svg_background_color", bgColor);
    actionRegistry->propertizeAction("svg_background_color", bgColor);
    m_richTextActions << bgColor;

    QWidgetAction *lineHeight = new QWidgetAction(this);
    lineHeight->setToolTip(i18n("Line height"));
    QDoubleSpinBox *spnLineHeight = new QDoubleSpinBox();
    spnLineHeight->setRange(0.0, 1000.0);
    spnLineHeight->setSingleStep(10.0);
    spnLineHeight->setSuffix("%");
    connect(spnLineHeight, SIGNAL(valueChanged(double)), SLOT(setLineHeight(double)));
    lineHeight->setDefaultWidget(spnLineHeight);
    actionCollection()->addAction("svg_line_height", lineHeight);
    m_richTextActions << lineHeight;
    actionRegistry->propertizeAction("svg_line_height", lineHeight);
}

void SvgTextEditor::enableRichTextActions(bool enable)
{
    Q_FOREACH(QAction *action, m_richTextActions) {
        action->setEnabled(enable);
    }
}
