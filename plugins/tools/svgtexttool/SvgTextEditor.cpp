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

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kactioncollection.h>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
#include <kguiitem.h>

#include <KoResourcePaths.h>
#include <KoSvgTextShape.h>
#include <KoSvgTextShapeMarkupConverter.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorPopupAction.h>

#include <kis_icon.h>
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <BasicXMLSyntaxHighlighter.h>

SvgTextEditor::SvgTextEditor(QWidget *parent, Qt::WindowFlags flags)
    : KXmlGuiWindow(parent, flags)
    , m_page(new QWidget(this))
    , m_shape(0)
{
    m_textEditorWidget.setupUi(m_page);
    setCentralWidget(m_page);

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


//    m_textEditorWidget.bnBold->setDefaultAction(makeBold);
//    QList<QAction*> textWeightActions;
//    textWeightActions.append(makeWeightLight);
//    textWeightActions.append(makeWeightNormal);
//    textWeightActions.append(makeWeightDemi);
//    textWeightActions.append(makeBold);
//    textWeightActions.append(makeWeightBlack);
//    QMenu *textWeight = new QMenu();
//    textWeight->addActions(textWeightActions);
//    m_textEditorWidget.bnBold->setMenu(textWeight);

//    QAction *makeItalic = new QAction(KisIconUtils::loadIcon("format-text-italic"), "Italic");
//    connect(makeItalic, SIGNAL(triggered(bool)), this, SLOT(setTextItalic()));
//    m_textEditorWidget.bnItalic->setDefaultAction(makeItalic);

//    QAction *makeUnderline = new QAction(KisIconUtils::loadIcon("format-text-underline"), "Underline");
//    connect(makeUnderline, SIGNAL(triggered(bool)), this, SLOT(setTextUnderline()));
//    m_textEditorWidget.bnUnderline->setDefaultAction(makeUnderline);
//    QAction *makeStrike = new QAction(KisIconUtils::loadIcon("format-text-strikethrough"), "Strikethrough");
//    connect(makeStrike, SIGNAL(triggered(bool)), this, SLOT(setTextStrikethrough()));
//    QAction *makeOverline = new QAction("Overline");
//    connect(makeOverline, SIGNAL(triggered(bool)), this, SLOT(setTextOverline()));
//    QList<QAction*> textDecorActions;
//    textDecorActions.append(makeUnderline);
//    textDecorActions.append(makeStrike);
//    textDecorActions.append(makeOverline);
//    QMenu *textDecoration = new QMenu();
//    textDecoration->addActions(textDecorActions);


//    connect(this, SIGNAL(okClicked()), SLOT(save()));
//    connect(m_textEditorWidget.bnUndo, SIGNAL(clicked()), m_textEditorWidget.svgTextEdit, SLOT(undo()));
//    connect(m_textEditorWidget.bnRedo, SIGNAL(clicked()), m_textEditorWidget.svgTextEdit, SLOT(redo()));
//    connect(m_textEditorWidget.bnCopy, SIGNAL(clicked()), m_textEditorWidget.svgTextEdit, SLOT(copy()));
//    connect(m_textEditorWidget.bnCut, SIGNAL(clicked()), m_textEditorWidget.svgTextEdit, SLOT(cut()));
//    connect(m_textEditorWidget.bnPaste, SIGNAL(clicked()), m_textEditorWidget.svgTextEdit, SLOT(paste()));
//    connect(m_textEditorWidget.textTab, SIGNAL(currentChanged(int)), this, SLOT(switchTextEditorTab()));
//    switchTextEditorTab();

//    m_textEditorWidget.bnUnderline->setMenu(textDecoration);
//    m_textEditorWidget.bnStrikethrough->setDefaultAction(makeStrike);

//    connect(m_textEditorWidget.bnTextFgColor, SIGNAL(changed(KoColor)), this, SLOT(setTextFill()));
//    connect(m_textEditorWidget.bnTextBgColor, SIGNAL(changed(KoColor)), this, SLOT(setTextStroke()));
//    //connect(widget.bnSuperscript, SIGNAL(clicked()), this, SLOT(setSuperscript()));
//    //connect(widget.bnSubscript, SIGNAL(clicked()), this, SLOT(setSubscript()));
//    connect(m_textEditorWidget.fontComboBox, SIGNAL(currentFontChanged(const QFont)), this, SLOT(setFont()));
//    connect(m_textEditorWidget.fontSize, SIGNAL(editingFinished()), this, SLOT(setSize()));



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
        if (converter.convertToSvg(&svg, &styles)) {
            //widget.svgTextEdit->setPlainText(QString("%1\n%2").arg(defs).arg(svg));
            m_textEditorWidget.svgTextEdit->setPlainText(svg);
        }
        else {
            qWarning() << "Could not get svg text from the shape:" << converter.errors() << converter.warnings();
        }

    }
}

void SvgTextEditor::save()
{
//    // We don't do defs or styles yet...
//    emit textUpdated(m_textEditorWidget.svgTextEdit->document()->toPlainText(), "");
//    hide();
}

void SvgTextEditor::switchTextEditorTab()
{
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        //first, make buttons checkable
//        m_textEditorWidget.bnBold->setCheckable(true);
//        m_textEditorWidget.bnItalic->setCheckable(true);
//        m_textEditorWidget.bnUnderline->setCheckable(true);
//        //then connec the cursor change to the checkformat();
//        connect(m_textEditorWidget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));

//        //implement svg to richtext here.
//    } else {
//        //first, make buttons uncheckable
//        m_textEditorWidget.bnBold->setCheckable(false);
//        m_textEditorWidget.bnItalic->setCheckable(false);
//        m_textEditorWidget.bnUnderline->setCheckable(false);
//        disconnect(m_textEditorWidget.richTextEdit, SIGNAL(cursorPositionChanged()), this, SLOT(checkFormat()));

//        //implement richtext to svg here.
//    }
}

void SvgTextEditor::checkFormat()
{
//    QTextCharFormat format = m_textEditorWidget.richTextEdit->textCursor().charFormat();
//    if (format.fontWeight()>QFont::Normal) {
//        m_textEditorWidget.bnBold->setChecked(true);
//    } else {
//        m_textEditorWidget.bnBold->setChecked(false);
//    }
//    m_textEditorWidget.bnItalic->setChecked(format.fontItalic());
//    m_textEditorWidget.bnUnderline->setChecked(format.fontUnderline());
//    //widget.fontComboBox->setCurrentFont(format.font());
//    m_textEditorWidget.fontSize->setValue(format.fontPointSize());
//    KoColor color(format.foreground().color(), KoColorSpaceRegistry::instance()->rgb8());
//    m_textEditorWidget.bnTextFgColor->setColor(color);
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

void SvgTextEditor::close()
{

}


void SvgTextEditor::undo()
{

}

void SvgTextEditor::redo()
{

}

void SvgTextEditor::cut()
{

}

void SvgTextEditor::copy()
{

}

void SvgTextEditor::paste()
{

}

void SvgTextEditor::selectAll()
{

}

void SvgTextEditor::deselect()
{

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


void SvgTextEditor::insertSpecialCharacter()
{

}


void SvgTextEditor::setTextBold(QFont::Weight weight)
{
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        QTextCharFormat format;
//        if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal && weight==QFont::Bold) {
//            format.setFontWeight(QFont::Normal);
//        } else {
//            format.setFontWeight(weight);
//        }
//        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
//    } else {
//        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//        if (cursor.hasSelection()) {
//            QString selectionModified = "<tspan style=\"font-weight:700;\">" + cursor.selectedText() + "</tspan>";
//            cursor.removeSelectedText();
//            cursor.insertText(selectionModified);
//        }
//    }
}

void SvgTextEditor::setTextWeightLight()
{
//    if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight()<QFont::Normal) {
//        setTextBold(QFont::Normal);
//    } else {
//        setTextBold(QFont::Light);
//    }
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
//    if (m_textEditorWidget.richTextEdit->textCursor().charFormat().fontWeight()>QFont::Normal) {
//        setTextBold(QFont::Normal);
//    } else {
//        setTextBold(QFont::Black);
//    }
}

void SvgTextEditor::setTextItalic(QFont::Style style)
{
//    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//    QString fontStyle = "inherit";
//    if (style == QFont::StyleItalic) {
//        fontStyle = "italic";
//    } else if(style == QFont::StyleOblique) {
//        fontStyle = "oblique";
//    }
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        QTextCharFormat format;
//        format.setFontItalic(!m_textEditorWidget.richTextEdit->textCursor().charFormat().fontItalic());
//        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
//    } else {
//        if (cursor.hasSelection()) {
//            QString selectionModified = "<tspan style=\"font-style:"+fontStyle+";\">" + cursor.selectedText() + "</tspan>";
//            cursor.removeSelectedText();
//            cursor.insertText(selectionModified);
//        }
//    }
}

void SvgTextEditor::setTextDecoration(KoSvgText::TextDecoration decor)
{
//    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//    QTextCharFormat currentFormat = m_textEditorWidget.richTextEdit->textCursor().charFormat();
//    QTextCharFormat format;
//    QString textDecoration = "inherit";
//    if (decor == KoSvgText::DecorationUnderline) {
//        textDecoration = "underline";
//        if (currentFormat.fontUnderline()) {
//        format.setFontUnderline(false);
//        } else {
//            format.setFontUnderline(true);
//        }
//        format.setFontOverline(false);
//        format.setFontStrikeOut(false);
//    } else if (decor == KoSvgText::DecorationLineThrough) {
//        textDecoration = "line-through";
//        format.setFontUnderline(false);
//        format.setFontOverline(false);
//        if (currentFormat.fontStrikeOut()) {
//        format.setFontStrikeOut(false);
//        } else {
//            format.setFontStrikeOut(true);
//        }
//    } else if (decor == KoSvgText::DecorationOverline) {
//        textDecoration = "overline";
//        format.setFontUnderline(false);
//        if (currentFormat.fontOverline()) {
//        format.setFontOverline(false);
//        } else {
//            format.setFontOverline(true);
//        }
//        format.setFontStrikeOut(false);
//    }
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
//    } else {
//        if (cursor.hasSelection()) {
//            QString selectionModified = "<tspan style=\"text-decoration:"+textDecoration+";\">" + cursor.selectedText() + "</tspan>";
//            cursor.removeSelectedText();
//            cursor.insertText(selectionModified);
//        }
//    }
}

void SvgTextEditor::setTextUnderline()
{
    setTextDecoration();
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

void SvgTextEditor::setFont(const QString &family)
{

}


void SvgTextEditor::setFontSize(const QString &size)
{

}

void SvgTextEditor::setFontColor(const KoColor &c)
{

}

void SvgTextEditor::setBackgroundColor(const KoColor &c)
{

}


void SvgTextEditor::setTextFill()
{
//    KoColor c = m_textEditorWidget.bnTextFgColor->color();
//    QColor color = c.toQColor();
//    QTextEdit t;
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        QTextCharFormat format;
//        format.setForeground(QBrush(color));
//        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
//    } else {
//        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//        if (cursor.hasSelection()) {
//            QString selectionModified = "<tspan fill=\""+color.name()+"\">" + cursor.selectedText() + "</tspan>";
//            cursor.removeSelectedText();
//            cursor.insertText(selectionModified);
//        }
//    }
}

void SvgTextEditor::setTextStroke()
{
//    KoColor c = m_textEditorWidget.bnTextBgColor->color();
//    QColor color = c.toQColor();
//    QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//    if (cursor.hasSelection()) {
//        QString selectionModified = "<tspan stroke=\""+color.name()+"\">" + cursor.selectedText() + "</tspan>";
//        cursor.removeSelectedText();
//        cursor.insertText(selectionModified);
//    }
}

void SvgTextEditor::setFont()
{
//    QString fontName = m_textEditorWidget.fontComboBox->currentFont().family();
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        QTextCharFormat format;
//        format.setFontFamily(fontName);
//        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
//    } else {
//        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//        if (cursor.hasSelection()) {
//            QString selectionModified = "<tspan style=\"font-family:"+fontName+";\">" + cursor.selectedText() + "</tspan>";
//            cursor.removeSelectedText();
//            cursor.insertText(selectionModified);
//        }
//    }
}

void SvgTextEditor::setSize()
{
//    QString fontSize = QString::number(m_textEditorWidget.fontSize->value());
//    if (m_textEditorWidget.textTab->currentIndex() == Richtext) {
//        QTextCharFormat format;
//        format.setFontPointSize((qreal)m_textEditorWidget.fontSize->value());
//        m_textEditorWidget.richTextEdit->mergeCurrentCharFormat(format);
//    } else {
//        QTextCursor cursor = m_textEditorWidget.svgTextEdit->textCursor();
//        if (cursor.hasSelection()) {
//            QString selectionModified = "<tspan style=\"font-size:"+fontSize+";\">" + cursor.selectedText() + "</tspan>";
//            cursor.removeSelectedText();
//            cursor.insertText(selectionModified);
//        }
//    }
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

void SvgTextEditor::createAction(const QString &name, const QString &text, const QString &icon, const char *member, const QKeySequence &shortcut)
{
    QAction *action = new QAction(KisIconUtils::loadIcon(icon), text, this);
    actionCollection()->setDefaultShortcut(action, shortcut);
    actionCollection()->addAction(name, action);
    QObject::connect(action, SIGNAL(triggered(bool)), this, member);
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
    createAction("insert_special_character",
                 i18n("Insert Special Character"),
                 "insert-special-character",
                 SLOT(insertSpecialCharacter()));

    // Format:
    createAction("weight_bold",
                 i18n("Bold"),
                 "format-text-bold",
                 SLOT(setTextBold()));

    createAction("italic",
                 i18n("Italic"),
                 "format-text-italic",
                 SLOT(setTextItalic()));

    createAction("underline",
                 i18n("Underline"),
                 "format-text-underline",
                 SLOT(setTextUnderline()));

    createAction("strike_through",
                 i18n("Strike-through"),
                 "format-text-strike-through",
                 SLOT(setTextStrikethrough()));

    createAction("superscript",
                 i18n("Superscript"),
                 "format-text-superscript",
                 SLOT(setTextSuperScript()));

    createAction("subscript",
                 i18n("Subscript"),
                 "format-text-subscript",
                 SLOT(setTextSubscript()));

    createAction("weight_light",
                 i18n("Light"),
                 "format-text-light",
                 SLOT(setTextWeightLight()));

    createAction("weight_normal",
                 i18n("Normal"),
                 "format-text-normal",
                 SLOT(setTextWeightNormal()));

    createAction("weight_Demi",
                 i18n("Demi"),
                 "format-text-demi",
                 SLOT(setTextWeightDemi()));

    createAction("weight_black",
                 i18n("Black"),
                 "format-text-black",
                 SLOT(setTextWeightBlack()));

    createAction("increase_text_size",
                 i18n("Larger"),
                 "increase-font-size",
                 SLOT(increaseTextSize()));

    createAction("decrease_text_size",
                 i18n("Smaller"),
                 "decrease-font-size",
                 SLOT(decreaseTextSize()));

    createAction("align_left",
                 i18n("Align Left"),
                 "align-left",
                 SLOT(alignLeft()));

    createAction("align_right",
                 i18n("Alight Right"),
                 "align-right",
                 SLOT(alignRight()));

    createAction("align_center",
                 i18n("Align Center"),
                 "align-center",
                 SLOT(alignCenter()));

    createAction("align_justified",
                 i18n("Alight Justified"),
                 "align-justified",
                 SLOT(alignJustified()));

    // Settings: configure toolbars
    createAction("options_shape_properties",
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

    KoColorPopupAction *fgColor = new KoColorPopupAction(this);
    fgColor->setCurrentColor(QColor(Qt::black));
    fgColor->setToolTip(i18n("Text Color"));
    connect(fgColor, SIGNAL(colorChanged(KoColor)), SLOT(setFontColor(KoColor)));
    actionCollection()->addAction("font_color", fgColor);

    KoColorPopupAction *bgColor = new KoColorPopupAction(this);
    bgColor->setCurrentColor(QColor(Qt::white));
    bgColor->setToolTip(i18n("Background Color"));
    connect(bgColor, SIGNAL(colorChanged(KoColor)), SLOT(setBackgroundColor(KoColor)));
    actionCollection()->addAction("background_color", bgColor);
}
