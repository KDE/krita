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

#include <QPainter>
#include <QVBoxLayout>
#include <QUrl>
#include <QPushButton>
#include <QDebug>
#include <QAction>
#include <QWidgetAction>
#include <QMenu>
#include <QTabWidget>
#include <QFontComboBox>
#include <QComboBox>
#include <QMessageBox>
#include <QBuffer>
#include <QSvgGenerator>
#include <QTextEdit>

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
#include <BasicXMLSyntaxHighlighter.h>

SvgTextEditor::SvgTextEditor(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags)
    , m_page(new QWidget(this))
    , m_charSelectDialog(new KoDialog(this))
{
    m_textEditorWidget.setupUi(m_page);
    setCentralWidget(m_page);

    KCharSelect *charSelector = new KCharSelect(m_charSelectDialog, 0, KCharSelect::AllGuiElements);
    m_charSelectDialog->setMainWidget(charSelector);
    connect(charSelector, SIGNAL(currentCharChanged(QChar)), SLOT(insertCharacter(QChar)));
    m_charSelectDialog->hide();
    m_charSelectDialog->setButtons(KoDialog::Close);

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
    setStandardToolBarMenuEnabled(true);
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    BasicXMLSyntaxHighlighter *hl = new BasicXMLSyntaxHighlighter(m_textEditorWidget.svgTextEdit);
    Q_UNUSED(hl);

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
}

SvgTextEditor::~SvgTextEditor()
{
    KConfigGroup g(KSharedConfig::openConfig(), "SvgTextTool");
    QByteArray ba = saveState();
    g.writeEntry("windowState", ba.toBase64());
}


QString svgToHtml(const QString &svg)
{
    qDebug() << ">>>>>>>>>>>>> svg to html" << svg;

//    QDomDocument svgDoc;
//    svgDoc.setContent(svg.toUtf8());
//    QDomElement docEl = svgDoc.documentElement();
//    QDomNode n = docEl.firstChild();
//    while (!n.isNull()) {
//        QDomElement e = n.toElement();
//        if (!e.isNull()) {
//            qDebug() << e.tagName() << e.text();
//        }
//        n = n.nextSibling();
//    }


    return svg;
}

QString htmlToSvg(const QString &html)
{
//    qDebug() << ">>>>>>>>>>>>> html to svg" << html;

//    QDomDocument htmlDoc;
//    htmlDoc.setContent(html.toUtf8());
//    QDomElement docEl = htmlDoc.documentElement();
//    QDomNode n = docEl.firstChild();
//    while (!n.isNull()) {
//        QDomElement e = n.toElement();
//        if (!e.isNull()) {
//            qDebug() << e.tagName() << e.text();
//        }
//        n = n.nextSibling();
//    }

    return html;
}

void SvgTextEditor::setShape(KoSvgTextShape *shape)
{
    m_shape = shape;
    if (m_shape) {
        QTextDocument *doc = m_shape->textDocument();
        m_textEditorWidget.richTextEdit->setHtml(doc->toHtml());
        delete doc;
        KoSvgTextShapeMarkupConverter converter(m_shape);
        QString svg;
        QString styles;
        if (converter.convertToSvg(&svg, &styles)) {
            //m_textEditorWidget.svgTextEdit->setPlainText(QString("%1\n%2").arg(defs).arg(svg));
            m_textEditorWidget.svgTextEdit->setPlainText(svg);

        }
        else {
            QMessageBox::warning(this, i18n("Conversion failed"), "Could not get svg text from the shape:\n" + converter.errors().join('\n') + "\n" + converter.warnings().join('\n'));
        }
    }
}

void SvgTextEditor::save()
{
    //    // We don't do defs or styles yet...
    emit textUpdated(m_textEditorWidget.svgTextEdit->document()->toPlainText(), "");
}

void SvgTextEditor::switchTextEditorTab()
{
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        //first, make buttons checkable
        enableRichTextActions(true);
        //then connect the cursor change to the checkformat();
        connect(m_textEditorWidget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));
        //m_textEditorWidget.richTextEdit->setHtml(svgToHtml(m_textEditorWidget.svgTextEdit->document()->toPlainText()));
        m_currentEditor = m_textEditorWidget.richTextEdit;
    } else {
        //first, make buttons uncheckable
        enableRichTextActions(false);
        disconnect(m_textEditorWidget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));

        //m_textEditorWidget.svgTextEdit->setPlainText(htmlToSvg(m_textEditorWidget.richTextEdit->document()->toHtml()));
        m_currentEditor = m_textEditorWidget.svgTextEdit;
    }
}

void SvgTextEditor::checkFormat()
{
    QTextCharFormat format = m_textEditorWidget.richTextEdit->textCursor().charFormat();
    if (format.fontWeight() > QFont::Normal) {
        actionCollection()->action("weight_bold")->setChecked(true);
    } else {
        actionCollection()->action("weight_bold")->setChecked(false);
    }
    actionCollection()->action("italic")->setChecked(format.fontItalic());
    actionCollection()->action("underline")->setChecked(format.fontUnderline());
    actionCollection()->action("strike_through")->setChecked(format.fontStrikeOut());

    qobject_cast<QFontComboBox*>(qobject_cast<QWidgetAction*>(actionCollection()->action("font"))->defaultWidget())->setCurrentFont(format.font());

    QComboBox *fontSizeCombo = qobject_cast<QComboBox*>(qobject_cast<QWidgetAction*>(actionCollection()->action("font_size"))->defaultWidget());
    fontSizeCombo->setCurrentIndex(QFontDatabase::standardSizes().indexOf(format.font().pointSize()));

    KoColor fg(format.foreground().color(), KoColorSpaceRegistry::instance()->rgb8());
    qobject_cast<KoColorPopupAction*>(actionCollection()->action("font_color"))->setCurrentColor(fg);

    KoColor bg(format.foreground().color(), KoColorSpaceRegistry::instance()->rgb8());
    qobject_cast<KoColorPopupAction*>(actionCollection()->action("background_color"))->setCurrentColor(bg);
}

void SvgTextEditor::openNew()
{

}

void SvgTextEditor::open()
{

}

void SvgTextEditor::saveAs()
{

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
    m_currentEditor->textCursor().clearSelection();
}

void SvgTextEditor::find()
{

}

void SvgTextEditor::findNext()
{

}

void SvgTextEditor::findPrev()
{

}

void SvgTextEditor::replace()
{

}


void SvgTextEditor::zoomOut()
{

}

void SvgTextEditor::zoomIn()
{

}

void SvgTextEditor::zoom()
{

}


void SvgTextEditor::showInsertSpecialCharacterDialog()
{
    m_charSelectDialog->setVisible(!m_charSelectDialog->isVisible());
}

void SvgTextEditor::insertCharacter(const QChar &c)
{
    m_currentEditor->textCursor().insertText(QString(c));
}


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
    if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal
            && m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight()<QFont::Normal) {
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

}

void SvgTextEditor::setTextSuperScript()
{

}

void SvgTextEditor::increaseTextSize()
{

}

void SvgTextEditor::decreaseTextSize()
{

}


void SvgTextEditor::alignLeft()
{

}

void SvgTextEditor::alignRight()
{

}

void SvgTextEditor::alignCenter()
{

}

void SvgTextEditor::alignJustified()
{

}

void SvgTextEditor::setShapeProperties()
{

}

void SvgTextEditor::slotConfigureToolbars()
{

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

void SvgTextEditor::setFont(const QString &fontName)
{
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontFamily(fontName);
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-family:"+fontName+";\">" + cursor.selectedText() + "</tspan>";
            cursor.removeSelectedText();
            cursor.insertText(selectionModified);
        }
    }
}

void SvgTextEditor::setFontSize(const QString &fontSize)
{
    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
        QTextCharFormat format;
        format.setFontPointSize((qreal)fontSize.toInt());
        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
    } else {
        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
        if (cursor.hasSelection()) {
            QString selectionModified = "<tspan style=\"font-size:"+fontSize+";\">" + cursor.selectedText() + "</tspan>";
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

QAction *SvgTextEditor::createAction(const QString &name, const QString &text, const QString &icon, const char *member, const QKeySequence &shortcut)
{
    QAction *action = new QAction(KisIconUtils::loadIcon(icon), text, this);
    actionCollection()->setDefaultShortcut(action, shortcut);
    actionCollection()->addAction(name, action);
    QObject::connect(action, SIGNAL(triggered(bool)), this, member);
    return action;
}



void SvgTextEditor::createActions()
{
    // File: new, open, save, save as, close
    KStandardAction::openNew(this, SLOT(openNew()), actionCollection());
    KStandardAction::open(this, SLOT(open()), actionCollection());
    KStandardAction::save(this, SLOT(save()), actionCollection());
    KStandardAction::saveAs(this, SLOT(saveAs()), actionCollection());
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
    KStandardAction::zoom(this, SLOT(zoom()), actionCollection());

    // Insert:
    QAction * insertAction = createAction("insert_special_character",
                                          i18n("Insert Special Character"),
                                          "insert-special-character",
                                          SLOT(showInsertSpecialCharacterDialog()));
    insertAction->setCheckable(true);
    insertAction->setChecked(false);

    // Format:
    m_richTextActions << createAction("weight_bold",
                                      i18n("Bold"),
                                      "format-text-bold",
                                      SLOT(setTextBold()));

    m_richTextActions << createAction("italic",
                                      i18n("Italic"),
                                      "format-text-italic",
                                      SLOT(setTextItalic()));

    m_richTextActions << createAction("underline",
                                      i18n("Underline"),
                                      "format-text-underline",
                                      SLOT(setTextUnderline()));

    m_richTextActions << createAction("strike_through",
                                      i18n("Strike-through"),
                                      "format-text-strike-through",
                                      SLOT(setTextStrikethrough()));

    m_richTextActions << createAction("superscript",
                                      i18n("Superscript"),
                                      "format-text-superscript",
                                      SLOT(setTextSuperScript()));

    m_richTextActions << createAction("subscript",
                                      i18n("Subscript"),
                                      "format-text-subscript",
                                      SLOT(setTextSubscript()));

    m_richTextActions << createAction("weight_light",
                                      i18n("Light"),
                                      "format-text-light",
                                      SLOT(setTextWeightLight()));

    m_richTextActions << createAction("weight_normal",
                                      i18n("Normal"),
                                      "format-text-normal",
                                      SLOT(setTextWeightNormal()));

    m_richTextActions << createAction("weight_Demi",
                                      i18n("Demi"),
                                      "format-text-demi",
                                      SLOT(setTextWeightDemi()));

    m_richTextActions << createAction("weight_black",
                                      i18n("Black"),
                                      "format-text-black",
                                      SLOT(setTextWeightBlack()));

    m_richTextActions << createAction("increase_text_size",
                                      i18n("Larger"),
                                      "increase-font-size",
                                      SLOT(increaseTextSize()));

    m_richTextActions << createAction("decrease_text_size",
                                      i18n("Smaller"),
                                      "decrease-font-size",
                                      SLOT(decreaseTextSize()));

    m_richTextActions << createAction("align_left",
                                      i18n("Align Left"),
                                      "align-left",
                                      SLOT(alignLeft()));

    m_richTextActions << createAction("align_right",
                                      i18n("Alight Right"),
                                      "align-right",
                                      SLOT(alignRight()));

    m_richTextActions << createAction("align_center",
                                      i18n("Align Center"),
                                      "align-center",
                                      SLOT(alignCenter()));

    m_richTextActions << createAction("align_justified",
                                      i18n("Alight Justified"),
                                      "align-justified",
                                      SLOT(alignJustified()));

    // Settings: configure toolbars
    m_richTextActions << createAction("options_shape_properties",
                                      i18n("Properties"),
                                      "settings",
                                      SLOT(setShapeProperties()));

    KStandardAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());

    QWidgetAction *fontComboAction = new QWidgetAction(this);
    fontComboAction->setToolTip(i18n("Font"));
    QFontComboBox *fontCombo = new QFontComboBox();
    connect(fontCombo, SIGNAL(activated(QString)), SLOT(setFont(QString)));
    fontComboAction->setDefaultWidget(fontCombo);
    actionCollection()->addAction("font", fontComboAction);
    m_richTextActions << fontComboAction;

    QWidgetAction *fontSizeAction = new QWidgetAction(this);
    fontSizeAction->setToolTip(i18n("Size"));
    QComboBox *fontSizeCombo = new QComboBox();
    Q_FOREACH (int size, QFontDatabase::standardSizes()) {
        fontSizeCombo->addItem(QString::number(size));
    }
    fontSizeCombo->setCurrentIndex(QFontDatabase::standardSizes().indexOf(QApplication::font().pointSize()));
    connect(fontSizeCombo, SIGNAL(activated(QString)), SLOT(setFontSize(QString)));
    fontSizeAction->setDefaultWidget(fontSizeCombo);
    actionCollection()->addAction("font_size", fontSizeAction);
    m_richTextActions << fontSizeAction;

    KoColorPopupAction *fgColor = new KoColorPopupAction(this);
    fgColor->setCurrentColor(QColor(Qt::black));
    fgColor->setToolTip(i18n("Text Color"));
    connect(fgColor, SIGNAL(colorChanged(KoColor)), SLOT(setFontColor(KoColor)));
    actionCollection()->addAction("font_color", fgColor);
    m_richTextActions << fgColor;

    KoColorPopupAction *bgColor = new KoColorPopupAction(this);
    bgColor->setCurrentColor(QColor(Qt::white));
    bgColor->setToolTip(i18n("Background Color"));
    connect(bgColor, SIGNAL(colorChanged(KoColor)), SLOT(setBackgroundColor(KoColor)));
    actionCollection()->addAction("background_color", bgColor);
    m_richTextActions << bgColor;
}

void SvgTextEditor::enableRichTextActions(bool enable)
{
    Q_FOREACH(QAction *action, m_richTextActions) {
        action->setEnabled(enable);
    }
}
