/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2008, 2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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

#include "TextTool.h"
#include "TextEditingPluginContainer.h"
#include "dialogs/SimpleCharacterWidget.h"
#include "dialogs/SimpleParagraphWidget.h"
#include "dialogs/SimpleTableWidget.h"
#include "dialogs/SimpleInsertWidget.h"
#include "dialogs/ParagraphSettingsDialog.h"
#include "dialogs/StyleManagerDialog.h"
#include "dialogs/InsertCharacter.h"
#include "dialogs/FontDia.h"
#include "dialogs/TableDialog.h"
#include "dialogs/SimpleTableWidget.h"
#include "commands/TextCutCommand.h"
#include "commands/AutoResizeCommand.h"
#include "commands/ChangeListLevelCommand.h"
#include "FontSizeAction.h"

#include <KoOdf.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoCanvasController.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoColorBackground.h>
#include <KoColorPopupAction.h>
#include <KoTextDocumentLayout.h>
#include <KoParagraphStyle.h>
#include <KoTextEditingPlugin.h>
#include <KoTextEditingRegistry.h>
#include <KoInlineTextObjectManager.h>
#include <KoStyleManager.h>
#include <KoTextOdfSaveHelper.h>
#include <KoTextDrag.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>
#include <KoListLevelProperties.h>
#include <KoTextLayoutRootArea.h>
//#include <ResizeTableCommand.h>

#include <kdebug.h>
#include <KIcon>
#include <KRun>
#include <KStandardShortcut>
#include <KFontChooser>
#include <KFontAction>
#include <KAction>
#include <KActionMenu>
#include <KMenu>
#include <KLocale>
#include <KStandardAction>
#include <KMimeType>
#include <KMessageBox>
#include <QTabWidget>
#include <QTextDocumentFragment>
#include <QToolTip>
#include <QSignalMapper>
#include <QGraphicsWidget>
#include <QLinearGradient>
#include <QBitmap>

#include <KoDocumentRdfBase.h>

class TextToolSelection : public KoToolSelection
{
public:

    TextToolSelection(QWeakPointer<KoTextEditor> editor)
        : KoToolSelection(0)
        , m_editor(editor)
    {
    }

    bool hasSelection()
    {
        if (!m_editor.isNull()) {
            return m_editor.data()->hasSelection();
        }
        return false;
    }

    QWeakPointer<KoTextEditor> m_editor;
};

static bool hit(const QKeySequence &input, KStandardShortcut::StandardShortcut shortcut)
{
    foreach (const QKeySequence & ks, KStandardShortcut::shortcut(shortcut).toList()) {
        if (input == ks)
            return true;
    }
    return false;
}

TextTool::TextTool(KoCanvasBase *canvas)
        : KoToolBase(canvas),
        m_textShape(0),
        m_textShapeData(0),
        m_changeTracker(0),
        m_allowActions(true),
        m_allowAddUndoCommand(true),
        m_allowResourceManagerUpdates(true),
        m_prevCursorPosition(-1),
        m_caretTimer(this),
        m_caretTimerState(true),
        m_currentCommand(0),
        m_currentCommandHasChildren(false),
        m_specialCharacterDocker(0),
        m_textTyping(false),
        m_textDeleting(false),
        m_changeTipTimer(this),
        m_changeTipCursorPos(0),
        m_delayedEnsureVisible(false),
        m_toolSelection(0)
        , m_tableDraggedOnce(false)
        , m_tablePenMode(false)
        ,m_lastImMicroFocus(QRectF(0,0,0,0))
{
    setTextMode(true);

    createActions();

    m_unit = canvas->resourceManager()->unitResource(KoCanvasResourceManager::Unit);
    m_textEditingPlugins = canvas->resourceManager()->
        resource(TextEditingPluginContainer::ResourceId).value<TextEditingPluginContainer*>();
    if (m_textEditingPlugins == 0) {
        m_textEditingPlugins = new TextEditingPluginContainer(canvas->resourceManager());
        QVariant variant;
        variant.setValue(m_textEditingPlugins);
        canvas->resourceManager()->setResource(TextEditingPluginContainer::ResourceId, variant);
    }

    foreach (KoTextEditingPlugin* plugin, m_textEditingPlugins->values()) {
        connect(plugin, SIGNAL(startMacro(const QString &)),
                this, SLOT(startMacro(const QString &)));
        connect(plugin, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
        QHash<QString, KAction*> actions = plugin->actions();
        QHash<QString, KAction*>::iterator i = actions.begin();
        while (i != actions.end()) {
            addAction(i.key(), i.value());
            ++i;
        }
    }

    // setup the context list.
    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(startTextEditingPlugin(QString)));
    QList<QAction*> list;
    list.append(this->action("format_font"));
    foreach (const QString &key, KoTextEditingRegistry::instance()->keys()) {
        KoTextEditingFactory *factory =  KoTextEditingRegistry::instance()->value(key);
        if (factory->showInMenu()) {
            KAction *a = new KAction(factory->title(), this);
            connect(a, SIGNAL(triggered()), signalMapper, SLOT(map()));
            signalMapper->setMapping(a, factory->id());
            list.append(a);
            addAction(QString("apply_%1").arg(factory->id()), a);
        }
    }
    setPopupActionList(list);

    connect(canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(shapeAddedToCanvas()));

    m_caretTimer.setInterval(500);
    connect(&m_caretTimer, SIGNAL(timeout()), this, SLOT(blinkCaret()));

    m_changeTipTimer.setInterval(500);
    m_changeTipTimer.setSingleShot(true);
    connect(&m_changeTipTimer, SIGNAL(timeout()), this, SLOT(showChangeTip()));
}

void TextTool::createActions()
{
    m_actionPasteAsText  = new KAction(KIcon("paste"), i18n("Paste As Text"), this);
    addAction("edit_paste_text", m_actionPasteAsText);
    m_actionPasteAsText->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_V);
    connect(m_actionPasteAsText, SIGNAL(triggered(bool)), this, SLOT(pasteAsText()));

    m_actionFormatBold  = new KAction(KIcon("format-text-bold"), i18n("Bold"), this);
    addAction("format_bold", m_actionFormatBold);
    m_actionFormatBold->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actionFormatBold->setCheckable(true);
    connect(m_actionFormatBold, SIGNAL(triggered(bool)), this, SLOT(bold(bool)));

    m_actionFormatItalic  = new KAction(KIcon("format-text-italic"), i18n("Italic"), this);
    addAction("format_italic", m_actionFormatItalic);
    m_actionFormatItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    m_actionFormatItalic->setCheckable(true);
    connect(m_actionFormatItalic, SIGNAL(triggered(bool)), this, SLOT(italic(bool)));

    m_actionFormatUnderline  = new KAction(KIcon("format-text-underline"), i18nc("Text formatting", "Underline"), this);
    addAction("format_underline", m_actionFormatUnderline);
    m_actionFormatUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    m_actionFormatUnderline->setCheckable(true);
    connect(m_actionFormatUnderline, SIGNAL(triggered(bool)), this, SLOT(underline(bool)));

    m_actionFormatStrikeOut  = new KAction(KIcon("format-text-strikethrough"), i18n("Strike Out"), this);
    addAction("format_strike", m_actionFormatStrikeOut);
    m_actionFormatStrikeOut->setCheckable(true);
    connect(m_actionFormatStrikeOut, SIGNAL(triggered(bool)), this, SLOT(strikeOut(bool)));

    QActionGroup *alignmentGroup = new QActionGroup(this);
    m_actionAlignLeft  = new KAction(KIcon("format-justify-left"), i18n("Align Left"), this);
    addAction("format_alignleft", m_actionAlignLeft);
    m_actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionAlignLeft->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignLeft);
    connect(m_actionAlignLeft, SIGNAL(triggered(bool)), this, SLOT(alignLeft()));

    m_actionAlignRight  = new KAction(KIcon("format-justify-right"), i18n("Align Right"), this);
    addAction("format_alignright", m_actionAlignRight);
    m_actionAlignRight->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignRight->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignRight);
    connect(m_actionAlignRight, SIGNAL(triggered(bool)), this, SLOT(alignRight()));

    m_actionAlignCenter  = new KAction(KIcon("format-justify-center"), i18n("Align Center"), this);
    addAction("format_aligncenter", m_actionAlignCenter);
    m_actionAlignCenter->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_C);
    m_actionAlignCenter->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignCenter);
    connect(m_actionAlignCenter, SIGNAL(triggered(bool)), this, SLOT(alignCenter()));

    m_actionAlignBlock  = new KAction(KIcon("format-justify-fill"), i18n("Align Block"), this);
    addAction("format_alignblock", m_actionAlignBlock);
    m_actionAlignBlock->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignBlock->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignBlock);
    connect(m_actionAlignBlock, SIGNAL(triggered(bool)), this, SLOT(alignBlock()));

    m_actionChangeDirection = new KAction(KIcon("format-text-direction-rtl"), i18n("Change text direction"), this);
    addAction("change_text_direction", m_actionChangeDirection);
    m_actionChangeDirection->setToolTip(i18n("Change writing direction"));
    m_actionChangeDirection->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_D);
    m_actionChangeDirection->setCheckable(true);
    connect(m_actionChangeDirection, SIGNAL(triggered()), this, SLOT(textDirectionChanged()));


    m_actionFormatSuper = new KAction(KIcon("format-text-superscript"), i18n("Superscript"), this);
    m_actionFormatSuper->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);
    addAction("format_super", m_actionFormatSuper);
    m_actionFormatSuper->setCheckable(true);
    connect(m_actionFormatSuper, SIGNAL(triggered(bool)), this, SLOT(superScript(bool)));

    m_actionFormatSub = new KAction(KIcon("format-text-subscript"), i18n("Subscript"), this);
    m_actionFormatSub->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_B);
    addAction("format_sub", m_actionFormatSub);
    m_actionFormatSub->setCheckable(true);
    connect(m_actionFormatSub, SIGNAL(triggered(bool)), this, SLOT(subScript(bool)));

    KAction *action = new KAction(
        KIcon(QApplication::isRightToLeft() ? "format-indent-less" : "format-indent-more"),
        i18n("Increase Indent"), this);
    addAction("format_increaseindent", action);
    connect(action, SIGNAL(triggered()), this, SLOT(increaseIndent()));

    m_actionFormatDecreaseIndent = new KAction(
        KIcon(QApplication::isRightToLeft() ? "format-indent-more" : "format-indent-less"),
        i18n("Decrease Indent"), this);
    addAction("format_decreaseindent", m_actionFormatDecreaseIndent);
    connect(m_actionFormatDecreaseIndent, SIGNAL(triggered()), this, SLOT(decreaseIndent()));

    action = new KAction(KIcon("format-list-unordered"),  i18n("Bullet list"), this);
    addAction("format_bulletlist", action);

    action = new KAction(KIcon("format-list-ordered"),  i18n("Numbered list"), this);
    addAction("format_numberlist", action);

    action = new KAction(i18n("Increase Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Greater);
    addAction("fontsizeup", action);
    connect(action, SIGNAL(triggered()), this, SLOT(increaseFontSize()));

    action = new KAction(i18n("Decrease Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Less);
    addAction("fontsizedown", action);
    connect(action, SIGNAL(triggered()), this, SLOT(decreaseFontSize()));

    m_actionFormatFontFamily = new KFontAction(KFontChooser::SmoothScalableFonts, this);
    addAction("format_fontfamily", m_actionFormatFontFamily);
    connect(m_actionFormatFontFamily, SIGNAL(triggered(const QString &)),
            this, SLOT(setFontFamily(const QString &)));

    m_variableMenu = new KActionMenu(i18n("Variable"), this);
    addAction("insert_variable", m_variableMenu);

    // ------------------- Actions with a key binding and no GUI item
    action  = new KAction(i18n("Insert Non-Breaking Space"), this);
    addAction("nonbreaking_space", action);
    action->setShortcut(Qt::CTRL + Qt::Key_Space);
    connect(action, SIGNAL(triggered()), this, SLOT(nonbreakingSpace()));

    action  = new KAction(i18n("Insert Non-Breaking Hyphen"), this);
    addAction("nonbreaking_hyphen", action);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Minus);
    connect(action, SIGNAL(triggered()), this, SLOT(nonbreakingHyphen()));

    action  = new KAction(i18n("Insert Index"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_T);
    addAction("insert_index", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertIndexMarker()));

    action  = new KAction(i18n("Insert Soft Hyphen"), this);
    addAction("soft_hyphen", action);
    //action->setShortcut(Qt::CTRL + Qt::Key_Minus); // TODO this one is also used for the kde-global zoom-out :(
    connect(action, SIGNAL(triggered()), this, SLOT(softHyphen()));

    action  = new KAction(i18n("Line Break"), this);
    addAction("line_break", action);
    action->setShortcut(Qt::SHIFT + Qt::Key_Return);
    connect(action, SIGNAL(triggered()), this, SLOT(lineBreak()));

    action  = new KAction(KIcon("insert-pagebreak"), i18n("Page Break"), this);
    addAction("insert_framebreak", action);
    action->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Return));
    connect(action, SIGNAL(triggered()), this, SLOT(insertFrameBreak()));
    action->setToolTip(i18n("Insert a page break"));
    action->setWhatsThis(i18n("All text after this point will be moved into the next page."));


    action  = new KAction(i18n("Font..."), this);
    addAction("format_font", action);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_F);
    action->setToolTip(i18n("Change character size, font, boldface, italics etc."));
    action->setWhatsThis(i18n("Change the attributes of the currently selected characters."));
    connect(action, SIGNAL(triggered()), this, SLOT(selectFont()));

    m_actionFormatFontSize = new FontSizeAction(i18n("Font Size"), this);
    addAction("format_fontsize", m_actionFormatFontSize);
    connect(m_actionFormatFontSize, SIGNAL(fontSizeChanged(qreal)), this, SLOT(setFontSize(qreal)));

    m_actionFormatTextColor = new KoColorPopupAction(this);
    m_actionFormatTextColor->setIcon(KIcon("format-text-color"));
    m_actionFormatTextColor->setToolTip(i18n("Text Color..."));
    addAction("format_textcolor", m_actionFormatTextColor);
    connect(m_actionFormatTextColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setTextColor(const KoColor &)));

    m_actionFormatBackgroundColor = new KoColorPopupAction(this);
    m_actionFormatBackgroundColor->setIcon(KIcon("format-fill-color"));
    m_actionFormatBackgroundColor->setToolTip(i18n("Background Color..."));
    m_actionFormatBackgroundColor->setText(i18n("Background"));
    addAction("format_backgroundcolor", m_actionFormatBackgroundColor);
    connect(m_actionFormatBackgroundColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setBackgroundColor(const KoColor &)));

    m_growWidthAction = new KAction(KIcon("zoom-fit-best"), i18n("Grow To Fit Width"), this);
    addAction("grow_to_fit_width", m_growWidthAction);
    m_growWidthAction->setCheckable(true);
    connect(m_growWidthAction, SIGNAL(triggered(bool)), this, SLOT(setGrowWidthToFit(bool)));

    m_growHeightAction = new KAction(KIcon("zoom-fit-best"), i18n("Grow To Fit Height"), this);
    addAction("grow_to_fit_height", m_growHeightAction);
    m_growHeightAction->setCheckable(true);
    connect(m_growHeightAction, SIGNAL(triggered(bool)), this, SLOT(setGrowHeightToFit(bool)));

    m_shrinkToFitAction = new KAction(KIcon("zoom-fit-best"), i18n("Shrink To Fit"), this);
    addAction("shrink_to_fit", m_shrinkToFitAction);
    m_shrinkToFitAction->setCheckable(true);
    connect(m_shrinkToFitAction, SIGNAL(triggered(bool)), this, SLOT(setShrinkToFit(bool)));

    action = new KAction(KIcon("insert-table"), i18n("Insert Custom..."), this);
    addAction("insert_table", action);
    action->setToolTip(i18n("Insert a table into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertTable()));

    action  = new KAction(KIcon("edit-table-insert-row-above"), i18n("Row Above"), this);
    action->setToolTip(i18n("Insert Row Above"));
    addAction("insert_tablerow_above", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableRowAbove()));

    action  = new KAction(KIcon("edit-table-insert-row-below"), i18n("Row Below"), this);
    action->setToolTip(i18n("Insert Row Below"));
    addAction("insert_tablerow_below", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableRowBelow()));

    action  = new KAction(KIcon("edit-table-insert-column-left"), i18n("Column Left"), this);
    action->setToolTip(i18n("Insert Column Left"));
    addAction("insert_tablecolumn_left", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableColumnLeft()));

    action  = new KAction(KIcon("edit-table-insert-column-right"), i18n("Column Right"), this);
    action->setToolTip(i18n("Insert Column Right"));
    addAction("insert_tablecolumn_right", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableColumnRight()));
    action  = new KAction(KIcon("edit-table-delete-column"), i18n("Column"), this);
    action->setToolTip(i18n("Delete Column"));
    addAction("delete_tablecolumn", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteTableColumn()));

    action  = new KAction(KIcon("edit-table-delete-row"), i18n("Row"), this);
    action->setToolTip(i18n("Delete Row"));
    addAction("delete_tablerow", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteTableRow()));

    action  = new KAction(KIcon("merge"), i18n("Merge Cells"), this);
    addAction("merge_tablecells", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(mergeTableCells()));

    action  = new KAction(KIcon("split"), i18n("Split Cells"), this);
    addAction("split_tablecells", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(splitTableCells()));

    action  = new KAction(KIcon("edit-table-insert-row-above"), i18n("Row Above"), this);
    action->setToolTip(i18n("Insert Row Above"));
    addAction("insert_tablerow_above", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableRowAbove()));

    action  = new KAction(KIcon("edit-table-insert-row-below"), i18n("Row Below"), this);
    action->setToolTip(i18n("Insert Row Below"));
    addAction("insert_tablerow_below", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableRowBelow()));

    action  = new KAction(KIcon("edit-table-insert-column-left"), i18n("Column Left"), this);
    action->setToolTip(i18n("Insert Column Left"));
    addAction("insert_tablecolumn_left", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableColumnLeft()));

    action  = new KAction(KIcon("edit-table-insert-column-right"), i18n("Column Right"), this);
    action->setToolTip(i18n("Insert Column Right"));
    addAction("insert_tablecolumn_right", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(insertTableColumnRight()));

    action  = new KAction(KIcon("edit-table-delete-column"), i18n("Column"), this);
    action->setToolTip(i18n("Delete Column"));
    addAction("delete_tablecolumn", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteTableColumn()));

    action  = new KAction(KIcon("edit-table-delete-row"), i18n("Row"), this);
    action->setToolTip(i18n("Delete Row"));
    addAction("delete_tablerow", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(deleteTableRow()));

    action  = new KAction(KIcon("edit-table-cell-merge"), i18n("Merge Cells"), this);
    addAction("merge_tablecells", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(mergeTableCells()));

    action  = new KAction(KIcon("edit-table-cell-split"), i18n("Split Cells"), this);
    addAction("split_tablecells", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(splitTableCells()));

    action = new KAction(i18n("Paragraph..."), this);
    addAction("format_paragraph", action);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_P);
    action->setToolTip(i18n("Change paragraph margins, text flow, borders, bullets, numbering etc."));
    action->setWhatsThis(i18n("Change paragraph margins, text flow, borders, bullets, numbering etc.<p>Select text in multiple paragraphs to change the formatting of all selected paragraphs.<p>If no text is selected, the paragraph where the cursor is located will be changed.</p>"));
    connect(action, SIGNAL(triggered()), this, SLOT(formatParagraph()));

    action = new KAction(i18n("Style Manager..."), this);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_S);
    action->setToolTip(i18n("Change attributes of styles"));
    action->setWhatsThis(i18n("Change font and paragraph attributes of styles.<p>Multiple styles can be changed using the dialog box."));
    addAction("format_stylist", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showStyleManager()));

    action = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    addAction("edit_select_all", action);

    action = new KAction(i18n("Special Character..."), this);
    action->setIcon(KIcon("character-set"));
    action->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_C);
    addAction("insert_specialchar", action);
    action->setToolTip(i18n("Insert one or more symbols or characters not found on the keyboard"));
    action->setWhatsThis(i18n("Insert one or more symbols or characters not found on the keyboard."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertSpecialCharacter()));

    action = new KAction(i18n("Repaint"), this);
    action->setIcon(KIcon("view-refresh"));
    addAction("repaint", action);
    connect(action, SIGNAL(triggered()), this, SLOT(relayoutContent()));

#ifndef NDEBUG
    action = new KAction("Paragraph Debug", this); // do NOT add i18n!
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_P);
    addAction("detailed_debug_paragraphs", action);
    connect(action, SIGNAL(triggered()), this, SLOT(debugTextDocument()));
    action = new KAction("Styles Debug", this); // do NOT add i18n!
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_S);
    addAction("detailed_debug_styles", action);
    connect(action, SIGNAL(triggered()), this, SLOT(debugTextStyles()));
#endif
}


#ifndef NDEBUG
#include "tests/MockShapes.h"
#include <kundo2stack.h>

TextTool::TextTool(MockCanvas *canvas)  // constructor for our unit tests;
    : KoToolBase(canvas),
    m_textShape(0),
    m_textShapeData(0),
    m_changeTracker(0),
    m_allowActions(true),
    m_allowAddUndoCommand(true),
    m_allowResourceManagerUpdates(true),
    m_prevCursorPosition(-1),
    m_caretTimer(this),
    m_caretTimerState(true),
    m_currentCommand(0),
    m_currentCommandHasChildren(false),
    m_specialCharacterDocker(0),
    m_textEditingPlugins(0),
    m_changeTipTimer(this),
    m_changeTipCursorPos(0)
    , m_delayedEnsureVisible(false)
    , m_tableDraggedOnce(false)
    , m_tablePenMode(false)
{
    // we could init some vars here, but we probably don't have to
    KGlobal::setLocale(new KLocale("en"));
    QTextDocument *document = new QTextDocument(); // this document is leaked

    KoInlineTextObjectManager *inlineManager = new KoInlineTextObjectManager();
    KoTextDocument(document).setInlineTextObjectManager(inlineManager);

    m_textEditor = new KoTextEditor(document);
    KoTextDocument(document).setTextEditor(m_textEditor.data());
    m_toolSelection = new TextToolSelection(m_textEditor);

    m_changeTracker = new KoChangeTracker();
    KoTextDocument(document).setChangeTracker(m_changeTracker);

    KoTextDocument(document).setUndoStack(new KUndo2Stack());
}
#endif

TextTool::~TextTool()
{
    delete m_toolSelection;
}

void TextTool::showChangeTip()
{
    if (!m_textShapeData || !m_changeTipCursorPos || !m_changeTracker->displayChanges())
        return;

    QTextCursor c(m_textShapeData->document());
    c.setPosition(m_changeTipCursorPos);
    if (m_changeTracker && m_changeTracker->containsInlineChanges(c.charFormat())) {
        KoChangeTrackerElement *element = m_changeTracker->elementById(c.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt());
        if (element->isEnabled()) {
            QString changeType;
            if (element->getChangeType() == KoGenChange::InsertChange)
                changeType = i18n("Insertion");
            else if (element->getChangeType() == KoGenChange::DeleteChange)
                changeType = i18n("Deletion");
            else
                changeType = i18n("Formatting");

            QString change = "<p align=center style=\'white-space:pre\' ><b>" + changeType + "</b><br/>";

            QString date = element->getDate();
            //Remove the T which separates the Data and Time.
            date[10] = ' ';
            change += element->getCreator() + " " + date + "</p>";

            int toolTipWidth = QFontMetrics(QToolTip::font()).boundingRect(element->getDate() + ' ' + element->getCreator()).width();
            m_changeTipPos.setX(m_changeTipPos.x() - toolTipWidth/2);

            QToolTip::showText(m_changeTipPos,change,canvas()->canvasWidget());

        }
    }
}

void TextTool::blinkCaret()
{
    if (!(canvas()->canvasWidget() ? canvas()->canvasWidget()->hasFocus() : canvas()->canvasItem()->hasFocus())) {
        m_caretTimer.stop();
        m_caretTimerState = false; // not visible.
    }
    else {
        m_caretTimerState = !m_caretTimerState;
    }
    repaintCaret();
}

void TextTool::relayoutContent()
{
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    foreach (KoTextLayoutRootArea *rootArea, lay->rootAreas()) {
        rootArea->setDirty();
    }
    lay->emitLayoutIsDirty();
}

void TextTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_textEditor.isNull())
        return;
    if (canvas()
            && (( canvas()->canvasWidget() && canvas()->canvasWidget()->hasFocus())
                  || (canvas()->canvasItem() && canvas()->canvasItem()->hasFocus())
               )
            && !m_caretTimer.isActive()) { // make sure we blink
        m_caretTimer.start();
        m_caretTimerState = true;
    }
    if (!m_caretTimerState)
        m_caretTimer.setInterval(500); // we set it lower during typing, so set it back to normal

    if (!m_textShapeData)
        return;
    if (m_textShapeData->isDirty())
        return;

    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);

    painter.save();
    QTransform shapeMatrix = m_textShape->absoluteTransformation(&converter);
    shapeMatrix.scale(zoomX, zoomY);
    shapeMatrix.translate(0, -m_textShapeData->documentOffset());

    // Possibly draw table dragging visual cues
    const qreal boxHeight = 20;
    if (m_tableDragInfo.tableHit == KoPointedAt::ColumnDivider) {
        QPointF anchorPos = m_tableDragInfo.tableDividerPos - QPointF(m_dx, 0.0);
        if (m_tableDragInfo.tableColumnDivider > 0) {
            //let's draw left
            qreal w = m_tableDragInfo.tableLeadSize - m_dx;
            QRectF rect(anchorPos - QPointF(w, 0.0), QSizeF(w, 0.0));
            QRectF drawRect(shapeMatrix.map(rect.topLeft()), shapeMatrix.map(rect.bottomRight()));
            drawRect.setHeight(boxHeight);
            drawRect.moveTop(drawRect.top() - 1.5 * boxHeight);
            QString label = m_unit.toUserStringValue(w);
            int labelWidth = QFontMetrics(QToolTip::font()).boundingRect(label).width();
            painter.fillRect(drawRect, QColor(64, 255, 64, 196));
            painter.setPen(QColor(0, 0, 0, 196));
            if (labelWidth + 10 < drawRect.width()) {
                QPointF centerLeft(drawRect.left(), drawRect.center().y());
                QPointF centerRight(drawRect.right(), drawRect.center().y());
                painter.drawLine(centerLeft, drawRect.center() - QPointF(labelWidth/2+5, 0.0));
                painter.drawLine(centerLeft, centerLeft + QPointF(7, -5));
                painter.drawLine(centerLeft, centerLeft + QPointF(7, 5));
                painter.drawLine(drawRect.center() + QPointF(labelWidth/2+5, 0.0), centerRight);
                painter.drawLine(centerRight, centerRight + QPointF(-7, -5));
                painter.drawLine(centerRight, centerRight + QPointF(-7, 5));
                painter.drawText(drawRect, Qt::AlignCenter, label);
            }
        }
        if (m_tableDragInfo.tableColumnDivider <  m_tableDragInfo.table->columns()) {
            //let's draw right
            qreal w = m_tableDragInfo.tableTrailSize + m_dx;
            QRectF rect(anchorPos, QSizeF(w, 0.0));
            QRectF drawRect(shapeMatrix.map(rect.topLeft()), shapeMatrix.map(rect.bottomRight()));
            drawRect.setHeight(boxHeight);
            drawRect.moveTop(drawRect.top() - 1.5 * boxHeight);
            QString label;
            int labelWidth;
            if (m_tableDragWithShift) {
                label = i18n("follows along");
                labelWidth = QFontMetrics(QToolTip::font()).boundingRect(label).width();
                drawRect.setWidth(2 * labelWidth);
                QLinearGradient g(drawRect.topLeft(), drawRect.topRight());
                g.setColorAt(0.6, QColor(255, 64, 64, 196));
                g.setColorAt(1.0, QColor(255, 64, 64, 0));
                QBrush brush(g);
                painter.fillRect(drawRect, brush);
            } else {
                label = m_unit.toUserStringValue(w);
                labelWidth = QFontMetrics(QToolTip::font()).boundingRect(label).width();
                drawRect.setHeight(boxHeight);
                painter.fillRect(drawRect, QColor(64, 255, 64, 196));
            }
            painter.setPen(QColor(0, 0, 0, 196));
            if (labelWidth + 10 < drawRect.width()) {
                QPointF centerLeft(drawRect.left(), drawRect.center().y());
                QPointF centerRight(drawRect.right(), drawRect.center().y());
                painter.drawLine(centerLeft, drawRect.center() - QPointF(labelWidth/2+5, 0.0));
                painter.drawLine(centerLeft, centerLeft + QPointF(7, -5));
                painter.drawLine(centerLeft, centerLeft + QPointF(7, 5));
                if (!m_tableDragWithShift) {
                    painter.drawLine(drawRect.center() + QPointF(labelWidth/2+5, 0.0), centerRight);
                    painter.drawLine(centerRight, centerRight + QPointF(-7, -5));
                    painter.drawLine(centerRight, centerRight + QPointF(-7, 5));
                }
                painter.drawText(drawRect, Qt::AlignCenter, label);
            }
            if (!m_tableDragWithShift) {
                // let's draw a helper text too
                label = i18n("Press shift to not resize this");
                labelWidth = QFontMetrics(QToolTip::font()).boundingRect(label).width();
                labelWidth += 10;
                //if (labelWidth < drawRect.width())
                {
                    drawRect.moveTop(drawRect.top() + boxHeight);
                    drawRect.moveLeft(drawRect.left() + (drawRect.width() - labelWidth)/2);
                    drawRect.setWidth(labelWidth);
                    painter.fillRect(drawRect, QColor(64, 255, 64, 196));
                    painter.drawText(drawRect, Qt::AlignCenter, label);
                }
            }
        }
    }
    // Possibly draw table dragging visual cues
    if (m_tableDragInfo.tableHit == KoPointedAt::RowDivider) {
        QPointF anchorPos = m_tableDragInfo.tableDividerPos - QPointF(0.0, m_dy);
        if (m_tableDragInfo.tableRowDivider > 0) {
            qreal h = m_tableDragInfo.tableLeadSize - m_dy;
            QRectF rect(anchorPos - QPointF(0.0, h), QSizeF(0.0, h));
            QRectF drawRect(shapeMatrix.map(rect.topLeft()), shapeMatrix.map(rect.bottomRight()));
            drawRect.setWidth(boxHeight);
            drawRect.moveLeft(drawRect.left() - 1.5 * boxHeight);
            QString label = m_unit.toUserStringValue(h);
            QRectF labelRect = QFontMetrics(QToolTip::font()).boundingRect(label);
            labelRect.setHeight(boxHeight);
            labelRect.setWidth(labelRect.width() + 10);
            labelRect.moveTopLeft(drawRect.center() - QPointF(labelRect.width(), labelRect.height())/2);
            painter.fillRect(drawRect, QColor(64, 255, 64, 196));
            painter.fillRect(labelRect, QColor(64, 255, 64, 196));
            painter.setPen(QColor(0, 0, 0, 196));
            if (labelRect.height() + 10 < drawRect.height()) {
                QPointF centerTop(drawRect.center().x(), drawRect.top());
                QPointF centerBottom(drawRect.center().x(), drawRect.bottom());
                painter.drawLine(centerTop, drawRect.center() - QPointF(0.0, labelRect.height()/2+5));
                painter.drawLine(centerTop, centerTop + QPointF(-5, 7));
                painter.drawLine(centerTop, centerTop + QPointF(5, 7));
                painter.drawLine(drawRect.center() + QPointF(0.0, labelRect.height()/2+5), centerBottom);
                painter.drawLine(centerBottom, centerBottom + QPointF(-5, -7));
                painter.drawLine(centerBottom, centerBottom + QPointF(5, -7));
            }
            painter.drawText(labelRect, Qt::AlignCenter, label);
        }
    }
    if (m_caretTimerState) {
        // Lets draw the caret ourselves, as the Qt method doesn't take cursor
        // charFormat into consideration.
        QTextBlock block = m_textEditor.data()->block();
        if (block.isValid()) {
            int posInParag = m_textEditor.data()->position() - block.position();
            if (posInParag <= block.layout()->preeditAreaPosition())
                posInParag += block.layout()->preeditAreaText().length();

            QTextLine tl = block.layout()->lineForTextPosition(m_textEditor.data()->position() - block.position());
            if (tl.isValid()) {
                painter.setRenderHint(QPainter::Antialiasing, false);
                QRectF rect = caretRect(m_textEditor.data()->cursor());
                QPointF baselinePoint;
                if (tl.ascent() > 0) {
                    QFontMetricsF fm(m_textEditor.data()->charFormat().font(), painter.device());
                    rect.setY(rect.y() + tl.ascent() - qMin(tl.ascent(), fm.ascent()));
                    rect.setHeight(qMin(tl.ascent(), fm.ascent()) + qMin(tl.descent(), fm.descent()));
                    baselinePoint = QPoint(rect.x(), rect.y() + tl.ascent());
                } else {
                    //line only filled with characters-without-size (eg anchors)
                    // layout will make sure line has height of block font
                    QFontMetricsF fm(block.charFormat().font(), painter.device());
                    rect.setHeight(fm.ascent() + fm.descent());
                    baselinePoint = QPoint(rect.x(), rect.y() + fm.ascent());
                }
                QRectF drawRect(shapeMatrix.map(rect.topLeft()), shapeMatrix.map(rect.bottomLeft()));
                drawRect.setWidth(2);
                painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
                painter.fillRect(drawRect, QColor(128, 255, 128));
                if (m_textEditor.data()->isEditProtected(true)) {
                    QRectF circleRect(shapeMatrix.map(baselinePoint),QSizeF(14, 14));
                    circleRect.translate(-7.5, -7.5);
                    QPen pen(Qt::red);
                    pen.setWidthF(2.0);
                    painter.setPen(Qt::NoPen);
                    painter.setPen(pen);
                    painter.setBrush(QBrush(QColor(255, 255, 255, 192)));
                    painter.setRenderHint(QPainter::Antialiasing, true);
                    painter.drawEllipse(circleRect);
                    painter.drawLine(circleRect.topLeft() + QPointF(2,2),
                                    circleRect.bottomRight() - QPointF(2,2));
                }
            }
        }
    }

    painter.restore();
}

void TextTool::updateSelectedShape(const QPointF &point)
{
    if (m_textShape && !m_textShape->boundingRect().contains(point)) {
        QRectF area(point, QSizeF(1, 1));
        if (m_textEditor.data()->hasSelection())
            repaintSelection();
        else
            repaintCaret();
        foreach (KoShape *shape, canvas()->shapeManager()->shapesAt(area, true)) {
            if (shape->isContentProtected())
                continue;
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            if (textShape) {
                KoTextShapeData *d = static_cast<KoTextShapeData*>(textShape->userData());
                const bool sameDocument = m_textShapeData ? d->document() == m_textShapeData->document() : false;
                m_textShape = textShape;
                if (sameDocument)
                    break; // stop looking.
            }
        }

        setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));

        // This is how we inform the rulers of the active range
        // For now we will not consider table cells, but just give the shape dimensions
        QVariant v;
        QRectF rect(QPoint(), m_textShape->size());
        rect = m_textShape->absoluteTransformation(0).mapRect(rect);
        v.setValue(rect);
        canvas()->resourceManager()->setResource(KoCanvasResourceManager::ActiveRange, v);
    }
}

void TextTool::mousePressEvent(KoPointerEvent *event)
{
    if (m_textEditor.isNull())
        return;

    // request the software keyboard, if any
    if (event->button() == Qt::LeftButton && qApp->autoSipEnabled()) {
        QStyle::RequestSoftwareInputPanel behavior = QStyle::RequestSoftwareInputPanel(qApp->style()->styleHint(QStyle::SH_RequestSoftwareInputPanel));
        // the two following bools just make it all a lot easier to read in the following if()
        // basically, we require a widget for this to work (passing NULL to QApplication::sendEvent
        // crashes) and there are three tests any one of which can be true to trigger the event
        const bool hasWidget = canvas()->canvasWidget();
        const bool hasItem = canvas()->canvasItem();
        if ((behavior == QStyle::RSIP_OnMouseClick && (hasWidget || hasItem)) ||
            (hasWidget && canvas()->canvasWidget()->hasFocus()) ||
            (hasItem && canvas()->canvasItem()->hasFocus())) {
            QEvent event(QEvent::RequestSoftwareInputPanel);
            if (hasWidget) {
                QApplication::sendEvent(canvas()->canvasWidget(), &event);
            } else {
                QApplication::sendEvent(canvas()->canvasItem(), &event);
            }
        }
    }

    if (m_textEditor.data()->hasSelection())
        repaintSelection(); // will erase selection
    else
        repaintCaret();

    updateSelectedShape(event->point);

    KoSelection *selection = canvas()->shapeManager()->selection();
    if (m_textShape && !selection->isSelected(m_textShape) && m_textShape->isSelectable()) {
        selection->deselectAll();
        selection->select(m_textShape);
    }

    bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
    KoPointedAt pointedAt = hitTest(event->point);
    m_tableDraggedOnce = false;
    if (pointedAt.position != -1) {
        m_tablePenMode = false;
        if(event->button() == Qt::LeftButton || !m_textEditor.data()->hasSelection()) {
            m_textEditor.data()->setPosition(pointedAt.position, shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);                useCursor(Qt::IBeamCursor);
        } else if (false /*within previous selection*/) {
            m_textEditor.data()->setPosition(pointedAt.position, shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        }
        m_tableDragInfo.tableHit = KoPointedAt::None;
        if (m_caretTimer.isActive()) { // make the caret not blink, (blinks again after first draw)
            m_caretTimer.stop();
            m_caretTimer.setInterval(50);
            m_caretTimer.start();
            m_caretTimerState = true; // turn caret instantly on on click
        }
    } else {
        if (event->button() == Qt::RightButton) {
            m_tablePenMode = false;
            KoTextEditingPlugin *plugin = m_textEditingPlugins->spellcheck();
            if (plugin)
                plugin->setCurrentCursorPosition(m_textShapeData->document(), -1);

            event->ignore();
        } else if (m_tablePenMode) {
            m_textEditor.data()->beginEditBlock(i18nc("(qtundo-format)", "Change Border Formatting"));
            if (pointedAt.tableHit == KoPointedAt::ColumnDivider) {
                if (pointedAt.tableColumnDivider < pointedAt.table->columns()) {
                    m_textEditor.data()->setTableBorderData(pointedAt.table,
                        pointedAt.tableRowDivider, pointedAt.tableColumnDivider,
                        KoBorder::Left, m_tablePenBorderData);
                }
                if (pointedAt.tableColumnDivider > 0) {
                    m_textEditor.data()->setTableBorderData(pointedAt.table,
                        pointedAt.tableRowDivider, pointedAt.tableColumnDivider - 1,
                        KoBorder::Right, m_tablePenBorderData);
                }
            } else if (pointedAt.tableHit == KoPointedAt::RowDivider) {
                if (pointedAt.tableRowDivider < pointedAt.table->rows()) {
                    m_textEditor.data()->setTableBorderData(pointedAt.table,
                        pointedAt.tableRowDivider, pointedAt.tableColumnDivider,
                        KoBorder::Top, m_tablePenBorderData);
                }
                if (pointedAt.tableRowDivider > 0) {
                    m_textEditor.data()->setTableBorderData(pointedAt.table,
                        pointedAt.tableRowDivider-1, pointedAt.tableColumnDivider,
                        KoBorder::Bottom, m_tablePenBorderData);
                }
            }
            m_textEditor.data()->endEditBlock();
        } else {
            m_tableDragInfo = pointedAt;
            m_tablePenMode = false;
        }
        return;
    }
    if (shiftPressed) // altered selection.
        repaintSelection();
    else
        repaintCaret();

    updateSelectionHandler();
    updateStyleManager();

    updateActions();

    //activate context-menu for spelling-suggestions
    if (event->button() == Qt::RightButton) {
        KoTextEditingPlugin *plugin = m_textEditingPlugins->spellcheck();
        if (plugin)
            plugin->setCurrentCursorPosition(m_textShapeData->document(), m_textEditor.data()->position());

        event->ignore();
    }

    if (event->button() ==  Qt::MidButton) { // Paste
        const QMimeData *data = QApplication::clipboard()->mimeData(QClipboard::Selection);

        // on windows we do not have data if we try to paste this selection
        if (data) {
            m_prevCursorPosition = m_textEditor.data()->position();
            m_textEditor.data()->paste(data,canvas()->shapeController(), canvas()->resourceManager());
            editingPluginEvents();
        }
    }
}

void TextTool::setShapeData(KoTextShapeData *data)
{
    bool docChanged = !data || !m_textShapeData || m_textShapeData->document() != data->document();
    if (m_textShapeData) {
        disconnect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    }
    m_textShapeData = data;
    if (!m_textShapeData)
        return;
    connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    if (docChanged) {
        if (!m_textEditor.isNull()) {
            disconnect(m_textEditor.data(), SIGNAL(textFormatChanged()), this, SLOT(updateActions()));
        }
        m_textEditor = KoTextDocument(m_textShapeData->document()).textEditor();
        Q_ASSERT(m_textEditor.data());
        if (!m_toolSelection) {
            m_toolSelection = new TextToolSelection(m_textEditor.data());
        }
        else {
            m_toolSelection->m_editor = m_textEditor.data();
        }

        m_variableMenu->menu()->clear();
        KoTextDocument document(m_textShapeData->document());
        foreach (QAction *action, document.inlineTextObjectManager()->createInsertVariableActions(canvas())) {
            m_variableMenu->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(returnFocusToCanvas()));
        }

        connect(m_textEditor.data(), SIGNAL(textFormatChanged()), this, SLOT(updateActions()));
    }
}

void TextTool::updateSelectionHandler()
{
    if (m_textEditor) {
        emit selectionChanged(m_textEditor.data()->hasSelection());
        if (m_textEditor.data()->hasSelection()) {
            QClipboard *clipboard = QApplication::clipboard();
            if (clipboard->supportsSelection())
                clipboard->setText(m_textEditor.data()->selectedText(), QClipboard::Selection);
        }
    }

    KoCanvasResourceManager *p = canvas()->resourceManager();
    m_allowResourceManagerUpdates = false;
    if (m_textEditor && m_textShapeData) {
        p->setResource(KoText::CurrentTextPosition, m_textEditor.data()->position());
        p->setResource(KoText::CurrentTextAnchor, m_textEditor.data()->anchor());
        QVariant variant;
        variant.setValue<void*>(m_textShapeData->document());
        p->setResource(KoText::CurrentTextDocument, variant);
    } else {
        p->clearResource(KoText::CurrentTextPosition);
        p->clearResource(KoText::CurrentTextAnchor);
        p->clearResource(KoText::CurrentTextDocument);
    }
    m_allowResourceManagerUpdates = true;
}

void TextTool::copy() const
{
    if (!m_textShapeData || m_textEditor.isNull() || !m_textEditor.data()->hasSelection())
        return;
    int from = m_textEditor.data()->position();
    int to = m_textEditor.data()->anchor();
    KoTextOdfSaveHelper saveHelper(m_textShapeData->document(), from, to);
    KoTextDrag drag;

    KoDocumentResourceManager *rm = 0;
    if (canvas()->shapeController()) {
        rm = canvas()->shapeController()->resourceManager();
    }
    if (rm && rm->hasResource(KoText::DocumentRdf)) {
        KoDocumentRdfBase *rdf = qobject_cast<KoDocumentRdfBase*>(rm->resource(KoText::DocumentRdf).value<QObject*>());
        if (rdf) {
            saveHelper.setRdfModel(rdf->model());
        }
    }

    drag.setOdf(KoOdf::mimeType(KoOdf::Text), saveHelper);
    QTextDocumentFragment fragment = m_textEditor.data()->selection();
    drag.setData("text/html", fragment.toHtml("utf-8").toUtf8());
    drag.setData("text/plain", fragment.toPlainText().toUtf8());
    drag.addToClipboard();
}

void TextTool::deleteSelection()
{
    m_textEditor.data()->deleteChar();
    editingPluginEvents();
}

bool TextTool::paste()
{
    const QMimeData *data = QApplication::clipboard()->mimeData(QClipboard::Clipboard);

    // on windows we do not have data if we try to paste this selection
    if (!data) return false;

    m_prevCursorPosition = m_textEditor.data()->position();
    m_textEditor.data()->paste(data, canvas()->shapeController());
    editingPluginEvents();
    return true;
}

void TextTool::cut()
{
    m_textEditor.data()->addCommand(new TextCutCommand(this));
}

QStringList TextTool::supportedPasteMimeTypes() const
{
    QStringList list;
    list << "text/plain" << "text/html" << "application/vnd.oasis.opendocument.text";
    return list;
}

KoPointedAt TextTool::hitTest(const QPointF & point) const
{
    if (!m_textShape || !m_textShapeData) {
        return KoPointedAt();
    }
    QPointF p = m_textShape->convertScreenPos(point);
    KoTextLayoutRootArea *rootArea = m_textShapeData->rootArea();
    return rootArea ? rootArea->hitTest(p, Qt::FuzzyHit) : KoPointedAt();
}

void TextTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != m_textShape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    if (event->modifiers() & Qt::ShiftModifier) {
        // When whift is pressed we behave as a single press
        return mousePressEvent(event);
    }

    m_textEditor.data()->select(QTextCursor::WordUnderCursor);

    repaintSelection();
    updateSelectionHandler();
}

void TextTool::mouseTripleClickEvent(KoPointerEvent *event)
{
    if (canvas()->shapeManager()->shapeAt(event->point) != m_textShape) {
        event->ignore(); // allow the event to be used by another
        return;
    }

    if (event->modifiers() & Qt::ShiftModifier) {
        // When whift is pressed we behave as a single press
        return mousePressEvent(event);
    }

    m_textEditor.data()->clearSelection();
    m_textEditor.data()->movePosition(QTextCursor::StartOfBlock);
    m_textEditor.data()->movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    repaintSelection();
    updateSelectionHandler();
}

void TextTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_changeTipPos = event->globalPos();

    if (event->buttons()) {
        updateSelectedShape(event->point);
    }

    m_changeTipTimer.stop();

    if (QToolTip::isVisible())
        QToolTip::hideText();

    KoPointedAt pointedAt = hitTest(event->point);

    if (event->buttons() == Qt::NoButton) {
        if (m_tablePenMode) {
            if (pointedAt.tableHit == KoPointedAt::ColumnDivider || pointedAt.tableHit == KoPointedAt::RowDivider) {
                useTableBorderCursor();
            } else {
                useCursor(Qt::IBeamCursor);
            }
            // do nothing else
            return;
        }

        if (!m_textShapeData || pointedAt.position < 0) {
            if (pointedAt.tableHit == KoPointedAt::ColumnDivider) {
                useCursor(Qt::SplitHCursor);
                m_draggingOrigin = event->point;
            } else if (pointedAt.tableHit == KoPointedAt::RowDivider) {
                if (pointedAt.tableRowDivider > 0) {
                    useCursor(Qt::SplitVCursor);
                    m_draggingOrigin = event->point;
                } else
                    useCursor(Qt::IBeamCursor);
            } else {
                useCursor(Qt::IBeamCursor);
            }
            return;
        }

        QTextCursor mouseOver(m_textShapeData->document());
        mouseOver.setPosition(pointedAt.position);

        if (m_changeTracker && m_changeTracker->containsInlineChanges(mouseOver.charFormat())) {
            m_changeTipTimer.start();
            m_changeTipCursorPos = pointedAt.position;
        }

        if (pointedAt.bookmark || !pointedAt.externalHRef.isEmpty()) {
            useCursor(Qt::PointingHandCursor);
            return;
        }

        // check if mouse pointer is over shape with hyperlink
        KoShape *selectedShape = canvas()->shapeManager()->shapeAt(event->point);
        if (selectedShape != 0 && selectedShape != m_textShape && selectedShape->hyperLink().size() != 0) {
            useCursor(Qt::PointingHandCursor);
            return;
        }

        useCursor(Qt::IBeamCursor);
        return;
    } else {
        if (m_tableDragInfo.tableHit == KoPointedAt::ColumnDivider) {
            m_tableDragWithShift = event->modifiers() & Qt::ShiftModifier;
            if(m_tableDraggedOnce) {
                canvas()->shapeController()->resourceManager()->undoStack()->undo();
            }
            KUndo2Command *topCmd = m_textEditor.data()->beginEditBlock(i18nc("(qtundo-format)", "Adjust Column Width"));
            m_dx = m_draggingOrigin.x() - event->point.x();
            if (m_tableDragInfo.tableColumnDivider < m_tableDragInfo.table->columns()
                    && m_tableDragInfo.tableTrailSize + m_dx < 0) {
                m_dx = -m_tableDragInfo.tableTrailSize;
            }
            if (m_tableDragInfo.tableColumnDivider > 0) {
                if (m_tableDragInfo.tableLeadSize - m_dx < 0) {
                    m_dx = m_tableDragInfo.tableLeadSize;
                }
                m_textEditor.data()->adjustTableColumnWidth(m_tableDragInfo.table,
                    m_tableDragInfo.tableColumnDivider - 1,
                    m_tableDragInfo.tableLeadSize - m_dx, topCmd);
            } else {
                m_textEditor.data()->adjustTableWidth(m_tableDragInfo.table, -m_dx, 0.0);
            }
            if (m_tableDragInfo.tableColumnDivider < m_tableDragInfo.table->columns()) {
                if (!m_tableDragWithShift) {
                    m_textEditor.data()->adjustTableColumnWidth(m_tableDragInfo.table,
                        m_tableDragInfo.tableColumnDivider,
                        m_tableDragInfo.tableTrailSize + m_dx, topCmd);
                }
            } else {
                m_tableDragWithShift = true; // act like shift pressed
            }
            if (m_tableDragWithShift) {
                m_textEditor.data()->adjustTableWidth(m_tableDragInfo.table, 0.0, m_dx);
            }
            m_textEditor.data()->endEditBlock();
            m_tableDragInfo.tableDividerPos.setY(m_textShape->convertScreenPos(event->point).y());
            if (m_tableDraggedOnce) {
                //we need to redraw like this so we update outside the textshape too
                if (canvas()->canvasWidget())
                    canvas()->canvasWidget()->update();
                if (canvas()->canvasItem())
                    canvas()->canvasItem()->update();
            }
            m_tableDraggedOnce = true;
        } else if (m_tableDragInfo.tableHit == KoPointedAt::RowDivider) {
            if(m_tableDraggedOnce) {
                canvas()->shapeController()->resourceManager()->undoStack()->undo();
            }
            if (m_tableDragInfo.tableRowDivider > 0) {
                KUndo2Command *topCmd = m_textEditor.data()->beginEditBlock(i18nc("(qtundo-format)", "Adjust Row Height"));
                m_dy = m_draggingOrigin.y() - event->point.y();

                if (m_tableDragInfo.tableLeadSize - m_dy < 0) {
                    m_dy = m_tableDragInfo.tableLeadSize;
                }

                m_textEditor.data()->adjustTableRowHeight(m_tableDragInfo.table,
                    m_tableDragInfo.tableRowDivider - 1,
                    m_tableDragInfo.tableLeadSize - m_dy, topCmd);

                m_textEditor.data()->endEditBlock();

                m_tableDragInfo.tableDividerPos.setX(m_textShape->convertScreenPos(event->point).x());
                if (m_tableDraggedOnce) {
                    //we need to redraw like this so we update outside the textshape too
                    if (canvas()->canvasWidget())
                        canvas()->canvasWidget()->update();
                    if (canvas()->canvasItem())
                        canvas()->canvasItem()->update();
                }
                m_tableDraggedOnce = true;
            }

        } else if (m_tablePenMode) {
            // do nothing
        } else {
            useCursor(Qt::IBeamCursor);
            if (pointedAt.position == m_textEditor.data()->position()) return;
            if (pointedAt.position >= 0) {
                if (m_textEditor.data()->hasSelection())
                    repaintSelection(); // will erase selection
                else
                    repaintCaret();

                m_textEditor.data()->setPosition(pointedAt.position, QTextCursor::KeepAnchor);

                if (m_textEditor.data()->hasSelection())
                    repaintSelection();
                else
                    repaintCaret();
            }
        }

        updateSelectionHandler();
    }
}

void TextTool::mouseReleaseEvent(KoPointerEvent *event)
{
    event->ignore();
    editingPluginEvents();

    m_tableDragInfo.tableHit = KoPointedAt::None;
    if (m_tableDraggedOnce) {
        m_tableDraggedOnce = false;
        //we need to redraw like this so we update outside the textshape too
        if (canvas()->canvasWidget())
            canvas()->canvasWidget()->update();
        if (canvas()->canvasItem())
            canvas()->canvasItem()->update();
    }

    if (!m_textShapeData)
        return;

    // check if mouse pointer is not over some shape with hyperlink
    KoShape *selectedShape = canvas()->shapeManager()->shapeAt(event->point);
    if (selectedShape != 0 && selectedShape != m_textShape && selectedShape->hyperLink().size() != 0) {
        QString url = selectedShape->hyperLink();
        runUrl(event, url);
        return;
    }

    KoPointedAt pointedAt = hitTest(event->point);

    // Is there an anchor here ?
    if (!m_textEditor.data()->hasSelection()) {
        if (pointedAt.bookmark) {
            m_textEditor.data()->setPosition(pointedAt.bookmark->position());
            ensureCursorVisible();
            event->accept();
            return;
        }
        if (!pointedAt.externalHRef.isEmpty()) {
            runUrl(event, pointedAt.externalHRef);
        }
    }
}

void TextTool::keyPressEvent(QKeyEvent *event)
{
    int destinationPosition = -1; // for those cases where the moveOperation is not relevant;
    QTextCursor::MoveOperation moveOperation = QTextCursor::NoMove;
    KoTextEditor *textEditor = m_textEditor.data();
    m_tablePenMode = false; // keypress always stops the table (border) pen mode
    Q_ASSERT(textEditor);
    if (event->key() == Qt::Key_Backspace) {
        if (!textEditor->hasSelection() && textEditor->block().textList()
            && (textEditor->position() == textEditor->block().position())
            && !(m_changeTracker && m_changeTracker->recordChanges())) {
            if (!textEditor->blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem)) {
                // backspace at beginning of numbered list item, makes it unnumbered
                textEditor->toggleListNumbering(false);
            } else {
                KoListLevelProperties llp;
                llp.setStyle(KoListStyle::None);
                llp.setLevel(0);
                // backspace on numbered, empty parag, removes numbering.
                textEditor->setListProperties(llp);
            }
        } else if (textEditor->position() > 0 || textEditor->hasSelection()) {
            if (!textEditor->hasSelection() && event->modifiers() & Qt::ControlModifier) { // delete prev word.
                textEditor->movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            }
            textEditor->deletePreviousChar();

            editingPluginEvents();
        }
    } else if ((event->key() == Qt::Key_Tab)
        && ((!textEditor->hasSelection() && (textEditor->position() == textEditor->block().position())) || (textEditor->block().document()->findBlock(textEditor->anchor()) != textEditor->block().document()->findBlock(textEditor->position()))) && textEditor->block().textList()) {
        ChangeListLevelCommand::CommandType type = ChangeListLevelCommand::IncreaseLevel;
        ChangeListLevelCommand *cll = new ChangeListLevelCommand(*textEditor->cursor(), type, 1);
        textEditor->addCommand(cll);
        editingPluginEvents();
    } else if ((event->key() == Qt::Key_Backtab)
        && ((!textEditor->hasSelection() && (textEditor->position() == textEditor->block().position())) || (textEditor->block().document()->findBlock(textEditor->anchor()) != textEditor->block().document()->findBlock(textEditor->position()))) && textEditor->block().textList() && !(m_changeTracker && m_changeTracker->recordChanges())) {
        ChangeListLevelCommand::CommandType type = ChangeListLevelCommand::DecreaseLevel;
        ChangeListLevelCommand *cll = new ChangeListLevelCommand(*textEditor->cursor(), type, 1);
        textEditor->addCommand(cll);
        editingPluginEvents();
    } else if (event->key() == Qt::Key_Delete) {
        if (!textEditor->hasSelection() && event->modifiers() & Qt::ControlModifier) {// delete next word.
            textEditor->movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        }
        // the event only gets through when the Del is not used in the app
        // if the app forwards Del then deleteSelection is used
        textEditor->deleteChar();
        editingPluginEvents();
    } else if ((event->key() == Qt::Key_Left) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Left;
    } else if ((event->key() == Qt::Key_Right) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Right;
    } else if ((event->key() == Qt::Key_Up) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Up;
    } else if ((event->key() == Qt::Key_Down) && (event->modifiers() & Qt::ControlModifier) == 0) {
        moveOperation = QTextCursor::Down;
    } else {
        // check for shortcuts.
        QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));
        if (hit(item, KStandardShortcut::Begin))
            // Goto beginning of the document. Default: Ctrl-Home
            destinationPosition = 0;
        else if (hit(item, KStandardShortcut::End)) {
            // Goto end of the document. Default: Ctrl-End
            if (m_textShapeData) {
                QTextBlock last = m_textShapeData->document()->lastBlock();
                destinationPosition = last.position() + last.length() - 1;
            }
        } else if (hit(item, KStandardShortcut::Prior)) { // page up
            // Scroll up one page. Default: Prior
            event->ignore(); // let app level actions handle it
            return;
        }
        else if (hit(item, KStandardShortcut::Next)) {
            // Scroll down one page. Default: Next
            event->ignore(); // let app level actions handle it
            return;
        }
        else if (hit(item, KStandardShortcut::BeginningOfLine))
            // Goto beginning of current line. Default: Home
            moveOperation = QTextCursor::StartOfLine;
        else if (hit(item, KStandardShortcut::EndOfLine))
            // Goto end of current line. Default: End
            moveOperation = QTextCursor::EndOfLine;
        else if (hit(item, KStandardShortcut::BackwardWord))
            moveOperation = QTextCursor::WordLeft;
        else if (hit(item, KStandardShortcut::ForwardWord))
            moveOperation = QTextCursor::WordRight;
#ifdef Q_WS_MAC
        // Don't reject "alt" key, it may be used for typing text on Mac OS
        else if ((event->modifiers() & Qt::ControlModifier) || event->text().length() == 0) {
#else
        else if ((event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) || event->text().length() == 0) {
#endif
            event->ignore();
            return;
        } else if ((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)) {
            textEditor->newLine();
            updateActions();
            editingPluginEvents();
        } else if ((event->key() == Qt::Key_Tab || !(event->text().length() == 1 && !event->text().at(0).isPrint()))) { // insert the text
            m_prevCursorPosition = textEditor->position();
            textEditor->insertText(event->text());
            editingPluginEvents();
        }
    }
    if (moveOperation != QTextCursor::NoMove || destinationPosition != -1) {
        useCursor(Qt::BlankCursor);
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if (textEditor->hasSelection())
            repaintSelection(); // will erase selection
        else
            repaintCaret();
        QTextBlockFormat format = textEditor->blockFormat();

        KoText::Direction dir = static_cast<KoText::Direction>(format.intProperty(KoParagraphStyle::TextProgressionDirection));
        bool isRtl;
        if (dir == KoText::AutoDirection)
            isRtl = textEditor->block().text().isRightToLeft();
        else
            isRtl =  dir == KoText::RightLeftTopBottom;

        if (isRtl) { // if RTL toggle direction of cursor movement.
            switch (moveOperation) {
            case QTextCursor::Left: moveOperation = QTextCursor::Right; break;
            case QTextCursor::Right: moveOperation = QTextCursor::Left; break;
            case QTextCursor::WordRight: moveOperation = QTextCursor::WordLeft; break;
            case QTextCursor::WordLeft: moveOperation = QTextCursor::WordRight; break;
            default: break;
            }
        }
        int prevPosition = textEditor->position();
        if (moveOperation != QTextCursor::NoMove)
            textEditor->movePosition(moveOperation,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        else
            textEditor->setPosition(destinationPosition,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if (moveOperation == QTextCursor::Down && prevPosition == textEditor->position()) {
            // change behavior a little big from Qt; at the bottom of the doc we go to the end of the doc
            textEditor->movePosition(QTextCursor::End,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        }
        if (shiftPressed) // altered selection.
            repaintSelection();
        else
            repaintCaret();
        updateActions();
        editingPluginEvents();
    }
    if (m_caretTimer.isActive()) { // make the caret not blink but decide on the action if its visible or not.
        m_caretTimer.stop();
        m_caretTimer.setInterval(50);
        m_caretTimer.start();
        m_caretTimerState = true; // turn caret on while typing
    }
    if (moveOperation != QTextCursor::NoMove)
        // this difference in handling is need to prevent leaving a trail of old cursors onscreen
        ensureCursorVisible();
    else
        m_delayedEnsureVisible = true;

    updateSelectionHandler();
}

QVariant TextTool::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (!textEditor || !m_textShapeData)
        return QVariant();

    switch (query) {
    case Qt::ImMicroFocus: {
        // The rectangle covering the area of the input cursor in widget coordinates.
        QRectF rect = caretRect(textEditor->cursor());
        rect.moveTop(rect.top() - m_textShapeData->documentOffset());
        QTransform shapeMatrix = m_textShape->absoluteTransformation(&converter);
        qreal zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        shapeMatrix.scale(zoomX, zoomY);
        rect = shapeMatrix.mapRect(rect);
        return rect.toRect();
    }
    case Qt::ImFont:
        // The currently used font for text input.
        return textEditor->charFormat().font();
    case Qt::ImCursorPosition:
        // The logical position of the cursor within the text surrounding the input area (see ImSurroundingText).
        return textEditor->position() - textEditor->block().position();
    case Qt::ImSurroundingText:
        // The plain text around the input area, for example the current paragraph.
        return textEditor->block().text();
    case Qt::ImCurrentSelection:
        // The currently selected text.
        return textEditor->selectedText();
    default:
        ; // Qt 4.6 adds ImMaximumTextLength and ImAnchorPosition
    }
    return QVariant();
}

void TextTool::inputMethodEvent(QInputMethodEvent *event)
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    if (event->replacementLength() > 0) {
        textEditor->setPosition(textEditor->position() + event->replacementStart());
        for (int i = event->replacementLength(); i > 0; --i) {
            textEditor->deleteChar();
        }
    }
    if (!event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
        // The cursor may reside in a different block before vs. after keyPressEvent.
        QTextBlock block = textEditor->block();
        QTextLayout *layout = block.layout();
        Q_ASSERT(layout);
        layout->setPreeditArea(-1, QString());
    } else {
        QTextBlock block = textEditor->block();
        QTextLayout *layout = block.layout();
        Q_ASSERT(layout);
        layout->setPreeditArea(textEditor->position() - block.position(), event->preeditString());
        const_cast<QTextDocument*>(textEditor->document())->markContentsDirty(textEditor->position(), 1);
    }
    event->accept();
}

void TextTool::ensureCursorVisible(bool moveView)
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (!textEditor || !m_textShapeData)
        return;

    const int position = textEditor->position();

    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    KoTextLayoutRootArea *rootArea = lay->rootAreaForPosition(position);
    if (rootArea && rootArea->associatedShape() && m_textShapeData->rootArea() != rootArea) {
        // If we have changed root area we need to update m_textShape and m_textShapeData
        m_textShape = static_cast<TextShape*>(rootArea->associatedShape());
        Q_ASSERT(m_textShape);
        disconnect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
        m_textShapeData = static_cast<KoTextShapeData*>(m_textShape->userData());
        Q_ASSERT(m_textShapeData);
        connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    }

    if (!moveView) {
        return;
    }

    QRectF cursorPos = caretRect(textEditor->cursor());
    if (! cursorPos.isValid()) { // paragraph is not yet layouted.
        // The number one usecase for this is when the user pressed enter.
        // try to do it on next caret blink
        m_delayedEnsureVisible = true;
    }
    cursorPos.moveTop(cursorPos.top() - m_textShapeData->documentOffset());
    canvas()->ensureVisible(m_textShape->absoluteTransformation(0).mapRect(cursorPos));
}

void TextTool::keyReleaseEvent(QKeyEvent *event)
{
    event->accept();
}

void TextTool::updateActions()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0) {
        return;
    }
    m_allowActions = false;

    //Update the characterStyle related GUI elements
    QTextCharFormat cf = textEditor->charFormat();
    m_actionFormatBold->setChecked(cf.fontWeight() > QFont::Normal);
    m_actionFormatItalic->setChecked(cf.fontItalic());
    m_actionFormatUnderline->setChecked(cf.intProperty(KoCharacterStyle::UnderlineType) != KoCharacterStyle::NoLineType);
    m_actionFormatStrikeOut->setChecked(cf.intProperty(KoCharacterStyle::StrikeOutType) != KoCharacterStyle::NoLineType);
    bool super = false, sub = false;
    switch (cf.verticalAlignment()) {
    case QTextCharFormat::AlignSuperScript:
        super = true;
        break;
    case QTextCharFormat::AlignSubScript:
        sub = true;
        break;
    default:;
    }
    m_actionFormatSuper->setChecked(super);
    m_actionFormatSub->setChecked(sub);
    m_actionFormatFontSize->setFontSize(cf.font().pointSizeF());
    m_actionFormatFontFamily->setFont(cf.font().family());

    KoTextShapeData::ResizeMethod resizemethod = KoTextShapeData::AutoResize;
    if(m_textShapeData) {
        resizemethod = m_textShapeData->resizeMethod();
    }
    m_shrinkToFitAction->setEnabled(resizemethod != KoTextShapeData::AutoResize);
    m_shrinkToFitAction->setChecked(resizemethod == KoTextShapeData::ShrinkToFitResize);

    m_growWidthAction->setEnabled(resizemethod != KoTextShapeData::AutoResize);
    m_growWidthAction->setChecked(resizemethod == KoTextShapeData::AutoGrowWidth || resizemethod == KoTextShapeData::AutoGrowWidthAndHeight);

    m_growHeightAction->setEnabled(resizemethod != KoTextShapeData::AutoResize);
    m_growHeightAction->setChecked(resizemethod == KoTextShapeData::AutoGrowHeight || resizemethod == KoTextShapeData::AutoGrowWidthAndHeight);

    //update paragraphStyle GUI element
    QTextBlockFormat bf = textEditor->blockFormat();
    if (bf.alignment() == Qt::AlignLeading || bf.alignment() == Qt::AlignTrailing) {
        bool revert = (textEditor->block().layout()->textOption().textDirection() == Qt::RightToLeft);
        if ((bf.alignment() == Qt::AlignLeading) ^ revert)
            m_actionAlignLeft->setChecked(true);
        else
            m_actionAlignRight->setChecked(true);
    } else if (bf.alignment() == Qt::AlignHCenter)
        m_actionAlignCenter->setChecked(true);
    if (bf.alignment() == Qt::AlignJustify)
        m_actionAlignBlock->setChecked(true);
    else if (bf.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute))
        m_actionAlignLeft->setChecked(true);
    else if (bf.alignment() == (Qt::AlignRight | Qt::AlignAbsolute))
        m_actionAlignRight->setChecked(true);

    m_actionFormatDecreaseIndent->setEnabled(textEditor->blockFormat().leftMargin() > 0.);

    m_allowActions = true;

    ///TODO if selection contains several different format
    emit blockChanged(textEditor->block());
    emit charFormatChanged(cf, textEditor->blockCharFormat());
    emit blockFormatChanged(bf);
}

void TextTool::updateStyleManager()
{
    if (!m_textShapeData)
        return;
    KoStyleManager *styleManager = KoTextDocument(m_textShapeData->document()).styleManager();
    emit styleManagerChanged(styleManager);

    //TODO move this to its own method
    m_changeTracker = KoTextDocument(m_textShapeData->document()).changeTracker();
}

void TextTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    m_caretTimer.start();
    m_caretTimerState = true;
    foreach (KoShape *shape, shapes) {
        m_textShape = dynamic_cast<TextShape*>(shape);
        if (m_textShape)
            break;
    }
    if (!m_textShape) { // none found
        emit done();
        // This is how we inform the rulers of the active range
        // No shape means no active range
        canvas()->resourceManager()->setResource(KoCanvasResourceManager::ActiveRange, QVariant(QRectF()));
        return;
    }

    // This is how we inform the rulers of the active range
    // For now we will not consider table cells, but just give the shape dimensions
    QVariant v;
    QRectF rect(QPoint(), m_textShape->size());
    rect = m_textShape->absoluteTransformation(0).mapRect(rect);
    v.setValue(rect);
    canvas()->resourceManager()->setResource(KoCanvasResourceManager::ActiveRange, v);
    setShapeData(static_cast<KoTextShapeData*>(m_textShape->userData()));
    useCursor(Qt::IBeamCursor);

    updateStyleManager();
    repaintSelection();
    updateSelectionHandler();
    updateActions();
    if (m_specialCharacterDocker)
        m_specialCharacterDocker->setEnabled(true);
}

void TextTool::deactivate()
{
    m_caretTimer.stop();
    m_caretTimerState = false;
    repaintCaret();
    m_textShape = 0;

    // This is how we inform the rulers of the active range
    // No shape means no active range
    canvas()->resourceManager()->setResource(KoCanvasResourceManager::ActiveRange, QVariant(QRectF()));

    setShapeData(0);

    updateSelectionHandler();
    if (m_specialCharacterDocker) {
        m_specialCharacterDocker->setEnabled(false);
        m_specialCharacterDocker->setVisible(false);
    }
}

void TextTool::repaintDecorations()
{
    if (m_textShapeData)
        repaintSelection();
}

void TextTool::repaintCaret()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (!textEditor || !m_textShapeData)
        return;

    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    KoTextLayoutRootArea *rootArea = lay->rootAreaForPosition(textEditor->position());

    if (rootArea) {
        // If we have changed root area we need to update m_textShape and m_textShapeData
        if (m_delayedEnsureVisible) {
            m_delayedEnsureVisible = false;
            ensureCursorVisible();
            return;
        }

        ensureCursorVisible(false); // ensures the various vars are updated

        TextShape *textShape = static_cast<TextShape*>(rootArea->associatedShape());
        if (!textShape)
            return;
        if (!textShape->textShapeData())
            return;

        QRectF repaintRect = caretRect(textEditor->cursor());
        repaintRect.moveTop(repaintRect.top() - textShape->textShapeData()->documentOffset());
        if (repaintRect.isValid()) {
            repaintRect = textShape->absoluteTransformation(0).mapRect(repaintRect);

            // Make sure there is enough space to show an icon
            QRectF iconSize = canvas()->viewConverter()->viewToDocument(QRect(0,0,16, 16));
            repaintRect.setX(repaintRect.x() - iconSize.width() / 2);
            repaintRect.setWidth(iconSize.width());
            repaintRect.moveTop(repaintRect.y() - iconSize.height() / 2);
            repaintRect.moveBottom(repaintRect.bottom() + iconSize.height() / 2);

            canvas()->updateCanvas(repaintRect);
        }
    }
}

void TextTool::repaintSelection()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor == 0)
        return;
    QTextCursor cursor = *textEditor->cursor();

    QList<TextShape *> shapes;
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(lay);
    foreach (KoShape* shape, lay->shapes()) {
        TextShape *textShape = dynamic_cast<TextShape*>(shape);
        if (textShape == 0) // when the shape is being deleted its no longer a TextShape but a KoShape
            continue;

        if (textShape->textShapeData()->isCursorVisible(&cursor)) {
            //Q_ASSERT(!shapes.contains(textShape));
            if (!shapes.contains(textShape))
                shapes.append(textShape);
        }
    }

    // loop over all shapes that contain the text and update per shape.
    QRectF repaintRect = textRect(cursor);
    foreach (TextShape *ts, shapes) {
        QRectF rect = repaintRect;
        rect.moveTop(rect.y() - ts->textShapeData()->documentOffset());
        rect = ts->absoluteTransformation(0).mapRect(rect);
        QRectF r = ts->boundingRect().intersected(rect);
        canvas()->updateCanvas(r);
    }
}

QRectF TextTool::caretRect(QTextCursor *cursor) const
{
    QTextCursor tmpCursor(*cursor);
    tmpCursor.setPosition(cursor->position()); // looses the anchor

    QRectF rect = textRect(tmpCursor);
    if (rect.size() == QSizeF(0,0)) {
        rect = m_lastImMicroFocus; // prevent block changed but layout not done
    } else {
        m_lastImMicroFocus = rect;
    }
    return rect;
}

QRectF TextTool::textRect(QTextCursor &cursor) const
{
    if (!m_textShapeData)
        return QRectF();
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_textShapeData->document()->documentLayout());
    return lay->selectionBoundingBox(cursor);
}

KoToolSelection* TextTool::selection()
{
    return m_toolSelection;
}

QList<QWidget *> TextTool::createOptionWidgets()
{
    QList<QWidget *> widgets;
    SimpleCharacterWidget *scw = new SimpleCharacterWidget(this, 0);
    SimpleParagraphWidget *spw = new SimpleParagraphWidget(this, 0);
    if (m_textEditor.data()) {
        //initialise the char- and par- widgets with the current block and formats.
        scw->setCurrentBlockFormat(m_textEditor.data()->blockFormat());
        scw->setCurrentFormat(m_textEditor.data()->charFormat(), m_textEditor.data()-> blockCharFormat());
        spw->setCurrentBlock(m_textEditor.data()->block());
        spw->setCurrentFormat(m_textEditor.data()->blockFormat());
    }
    SimpleTableWidget *stw = new SimpleTableWidget(this, 0);
    SimpleInsertWidget *siw = new SimpleInsertWidget(this, 0);

    // Connect to/with simple character widget (docker)
    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), scw, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(charFormatChanged(QTextCharFormat, QTextCharFormat)), scw, SLOT(setCurrentFormat(QTextCharFormat, QTextCharFormat)));
    connect(this, SIGNAL(blockFormatChanged(QTextBlockFormat)), scw, SLOT(setCurrentBlockFormat(QTextBlockFormat)));
    connect(scw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));
    connect(scw, SIGNAL(characterStyleSelected(KoCharacterStyle *)), this, SLOT(setStyle(KoCharacterStyle*)));
    connect(scw, SIGNAL(newStyleRequested(QString)), this, SLOT(createStyleFromCurrentCharFormat(QString)));
    connect(scw, SIGNAL(showStyleManager(int)), this, SLOT(showStyleManager(int)));


    // Connect to/with simple paragraph widget (docker)
    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), spw, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(blockChanged(const QTextBlock&)), spw, SLOT(setCurrentBlock(const QTextBlock&)));
    connect(this, SIGNAL(blockFormatChanged(QTextBlockFormat)), spw, SLOT(setCurrentFormat(QTextBlockFormat)));
    connect(spw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));
    connect(spw, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SLOT(setStyle(KoParagraphStyle*)));
    connect(spw, SIGNAL(newStyleRequested(QString)), this, SLOT(createStyleFromCurrentBlockFormat(QString)));
    connect(spw, SIGNAL(showStyleManager(int)), this, SLOT(showStyleManager(int)));

    // Connect to/with simple table widget (docker)
    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), stw, SLOT(setStyleManager(KoStyleManager *)));
    connect(stw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));
    connect(stw, SIGNAL(tableBorderDataUpdated(const KoBorder::BorderData &)), this, SLOT(setTableBorderData(const KoBorder::BorderData &)));

    // Connect to/with simple insert widget (docker)
    connect(siw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));
    connect(siw, SIGNAL(insertTableQuick(int, int)), this, SLOT(insertTableQuick(int, int)));

    updateStyleManager();
    if (m_textShape) {
        updateActions();
    }
    scw->setWindowTitle(i18n("Character"));
    widgets.append(scw);
    spw->setWindowTitle(i18n("Paragraph"));
    widgets.append(spw);
    stw->setWindowTitle(i18n("Table"));
    widgets.append(stw);
    siw->setWindowTitle(i18n("Insert"));
    widgets.append(siw);
    return widgets;
}

void TextTool::returnFocusToCanvas()
{
    canvas()->canvasWidget()->setFocus();
}

void TextTool::startEditing(KUndo2Command* command)
{
    m_currentCommand = command;
    m_currentCommandHasChildren = true;
}

void TextTool::stopEditing()
{
    m_currentCommand = 0;
    m_currentCommandHasChildren = false;
}

void TextTool::pasteAsText()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (!textEditor) return;

    const QMimeData *data = QApplication::clipboard()->mimeData(QClipboard::Clipboard);
    // on windows we do not have data if we try to paste this selection
    if (!data) return;

    m_prevCursorPosition = m_textEditor.data()->position();
    textEditor->paste(data, canvas()->shapeController(), true);
    editingPluginEvents();
}

void TextTool::bold(bool bold)
{
    m_textEditor.data()->bold(bold);
}

void TextTool::italic(bool italic)
{
    m_textEditor.data()->italic(italic);
}

void TextTool::underline(bool underline)
{
    m_textEditor.data()->underline(underline);
}

void TextTool::strikeOut(bool strikeOut)
{
    m_textEditor.data()->strikeOut(strikeOut);
}

void TextTool::nonbreakingSpace()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(Qt::Key_nobreakspace)));
}

void TextTool::nonbreakingHyphen()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(0x2013)));
}

void TextTool::softHyphen()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(Qt::Key_hyphen)));
}

void TextTool::lineBreak()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->insertText(QString(QChar(0x2028)));
}

void TextTool::alignLeft()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setHorizontalTextAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
}

void TextTool::alignRight()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setHorizontalTextAlignment(Qt::AlignRight | Qt::AlignAbsolute);
}

void TextTool::alignCenter()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setHorizontalTextAlignment(Qt::AlignHCenter);
}

void TextTool::alignBlock()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setHorizontalTextAlignment(Qt::AlignJustify);
}

void TextTool::superScript(bool on)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    if (on)
        m_actionFormatSub->setChecked(false);
    m_textEditor.data()->setVerticalTextAlignment(on ? Qt::AlignTop : Qt::AlignVCenter);
}

void TextTool::subScript(bool on)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    if (on)
        m_actionFormatSuper->setChecked(false);
    m_textEditor.data()->setVerticalTextAlignment(on ? Qt::AlignBottom : Qt::AlignVCenter);
}

void TextTool::increaseIndent()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->increaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_textEditor.data()->blockFormat().leftMargin() > 0.);
}

void TextTool::decreaseIndent()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->decreaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_textEditor.data()->blockFormat().leftMargin() > 0.);
}

void TextTool::decreaseFontSize()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->decreaseFontSize();
}

void TextTool::increaseFontSize()
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->increaseFontSize();
}

void TextTool::setFontFamily(const QString &font)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setFontFamily(font);
}

void TextTool::setFontSize (qreal size)
{
    if (!m_allowActions || !m_textEditor.data()) return;
    m_textEditor.data()->setFontSize(size);
}

void TextTool::insertIndexMarker()
{
    // TODO handle result when we figure out how to report errors from a tool.
    m_textEditor.data()->insertIndexMarker();
}

void TextTool::insertFrameBreak()
{
    m_textEditor.data()->insertFrameBreak();

    ensureCursorVisible();
    m_delayedEnsureVisible = true;
}

void TextTool::setStyle(KoCharacterStyle *style)
{
    KoCharacterStyle *charStyle = style;
    //if the given KoCharacterStyle is null, set the KoParagraphStyle character properties
    if (!charStyle){
        charStyle = static_cast<KoCharacterStyle*>(KoTextDocument(m_textShapeData->document()).styleManager()->paragraphStyle(m_textEditor.data()->blockFormat().intProperty(KoParagraphStyle::StyleId)));
    }
    if (charStyle) {
        m_textEditor.data()->setStyle(charStyle);
        updateActions();
    }
}

void TextTool::setStyle(KoParagraphStyle *style)
{
    m_textEditor.data()->setStyle(style);
    updateActions();
}

void TextTool::insertTable()
{
    TableDialog *dia = new TableDialog(0);
    if (dia->exec() == TableDialog::Accepted)
        m_textEditor.data()->insertTable(dia->rows(), dia->columns());
    delete dia;
}

void TextTool::insertTableQuick(int rows, int columns)
{
    m_textEditor.data()->insertTable(rows, columns);
}

void TextTool::insertTableRowAbove()
{
    m_textEditor.data()->insertTableRowAbove();
}

void TextTool::insertTableRowBelow()
{
    m_textEditor.data()->insertTableRowBelow();
}

void TextTool::insertTableColumnLeft()
{
    m_textEditor.data()->insertTableColumnLeft();
}

void TextTool::insertTableColumnRight()
{
    m_textEditor.data()->insertTableColumnRight();
}

void TextTool::deleteTableColumn()
{
    m_textEditor.data()->deleteTableColumn();
}

void TextTool::deleteTableRow()
{
    m_textEditor.data()->deleteTableRow();
}

void TextTool::mergeTableCells()
{
    m_textEditor.data()->mergeTableCells();
}

void TextTool::splitTableCells()
{
    m_textEditor.data()->splitTableCells();
}

void TextTool::useTableBorderCursor()
{
    static unsigned char data[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x68, 0x00,
        0x00, 0x00, 0xf4, 0x00, 0x00, 0x00, 0xfa, 0x00, 0x00, 0x00, 0xfd, 0x00,
        0x00, 0x80, 0x7e, 0x00, 0x00, 0x40, 0x3f, 0x00, 0x00, 0xa0, 0x1f, 0x00,
        0x00, 0xd0, 0x0f, 0x00, 0x00, 0xe8, 0x07, 0x00, 0x00, 0xf4, 0x03, 0x00,
        0x00, 0xe4, 0x01, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x80, 0x41, 0x00, 0x00,
        0x40, 0x32, 0x00, 0x00, 0xa0, 0x0f, 0x00, 0x00, 0xd0, 0x0f, 0x00, 0x00,
        0xd0, 0x0f, 0x00, 0x00, 0xe8, 0x07, 0x00, 0x00, 0xf4, 0x01, 0x00, 0x00,
        0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    QBitmap result(32, 32);
    result.fill(Qt::color0);
    QPainter painter(&result);
    painter.drawPixmap(0, 0, QBitmap::fromData(QSize(25, 23), data));
    QBitmap brushMask = result.createHeuristicMask(false);

    useCursor(QCursor(result, brushMask, 1, 21));
}

void TextTool::setTableBorderData(const KoBorder::BorderData &data)
{
    m_tablePenMode = true;
    m_tablePenBorderData = data;
}

void TextTool::formatParagraph()
{
    ParagraphSettingsDialog *dia = new ParagraphSettingsDialog(this, m_textEditor.data());
    dia->setUnit(canvas()->unit());

    dia->exec();
    delete dia;
    returnFocusToCanvas();
}

void TextTool::testSlot(bool on)
{
    kDebug(32500) << "signal received. bool:" << on;
}

void TextTool::selectAll()
{
    KoTextEditor *textEditor = m_textEditor.data();
    if (!textEditor || !m_textShapeData)
        return;
    const int selectionLength = qAbs(textEditor->position() - textEditor->anchor());
    QTextBlock lastBlock = m_textShapeData->document()->lastBlock();
    textEditor->setPosition(lastBlock.position() + lastBlock.length() - 1);
    textEditor->setPosition(0, QTextCursor::KeepAnchor);
    repaintSelection();
    if (selectionLength != qAbs(textEditor->position() - textEditor->anchor())) // it actually changed
        emit selectionChanged(true);
}

void TextTool::startMacro(const QString &title)
{
    if (title != i18n("Key Press") && title !=i18n("Autocorrection")) //dirty hack while waiting for refactor of text editing
        m_textTyping = false;
    else
        m_textTyping = true;

    if (title != i18n("Delete") && title != i18n("Autocorrection")) //same dirty hack as above
        m_textDeleting = false;
    else
        m_textDeleting = true;

    if (m_currentCommand) return;

    class MacroCommand : public KUndo2Command
    {
    public:
        MacroCommand(const QString &title) : KUndo2Command(title), m_first(true) {}
        virtual void redo() {
            if (! m_first)
                KUndo2Command::redo();
            m_first = false;
        }
        virtual bool mergeWith(const KUndo2Command *) {
            return false;
        }
        bool m_first;
    };

    m_currentCommand = new MacroCommand(title);
    m_currentCommandHasChildren = false;
}

void TextTool::stopMacro()
{
    if (!m_currentCommand)
        return;
    if (! m_currentCommandHasChildren)
        delete m_currentCommand;
    m_currentCommand = 0;
}

void TextTool::showStyleManager(int styleId)
{
    if (!m_textShapeData)
        return;
    KoStyleManager *styleManager = KoTextDocument(m_textShapeData->document()).styleManager();
    Q_ASSERT(styleManager);
    if (!styleManager)
        return;  //don't crash
    StyleManagerDialog *dia = new StyleManagerDialog(canvas()->canvasWidget());
    dia->setStyleManager(styleManager);
    dia->setUnit(canvas()->unit());

    KoParagraphStyle *paragraphStyle = styleManager->paragraphStyle(styleId);
    if (paragraphStyle) {
        dia->setParagraphStyle(paragraphStyle);
    }
    KoCharacterStyle *characterStyle = styleManager->characterStyle(styleId);
    if (characterStyle) {
        dia->setCharacterStyle(characterStyle);
    }
    dia->show();
}

void TextTool::startTextEditingPlugin(const QString &pluginId)
{
    KoTextEditingPlugin *plugin = m_textEditingPlugins->plugin(pluginId);
    if (plugin) {
        if (m_textEditor.data()->hasSelection()) {
            int from = m_textEditor.data()->position();
            int to = m_textEditor.data()->anchor();
            if (from > to) // make sure we call the plugin consistently
                qSwap(from, to);
            plugin->checkSection(m_textShapeData->document(), from, to);
        } else
            plugin->finishedWord(m_textShapeData->document(), m_textEditor.data()->position());
    }
}

void TextTool::resourceChanged(int key, const QVariant &var)
{
    if (m_textEditor.isNull())
        return;
    if (!m_textShapeData)
        return;
    if (m_allowResourceManagerUpdates == false)
        return;
    if (key == KoText::CurrentTextPosition) {
        repaintSelection();
        m_textEditor.data()->setPosition(var.toInt());
        ensureCursorVisible();
    } else if (key == KoText::CurrentTextAnchor) {
        repaintSelection();
        int pos = m_textEditor.data()->position();
        m_textEditor.data()->setPosition(var.toInt());
        m_textEditor.data()->setPosition(pos, QTextCursor::KeepAnchor);
    } else if (key == KoCanvasResourceManager::Unit) {
        m_unit = var.value<KoUnit>();
    } else return;

    repaintSelection();
}

void TextTool::insertSpecialCharacter()
{
    if (m_specialCharacterDocker == 0) {
        m_specialCharacterDocker = new InsertCharacter(canvas()->canvasWidget());
        connect(m_specialCharacterDocker, SIGNAL(insertCharacter(const QString&)),
                this, SLOT(insertString(const QString&)));
    }

    m_specialCharacterDocker->show();
}

void TextTool::insertString(const QString& string)
{
    m_textEditor.data()->insertText(string);
    returnFocusToCanvas();
}

void TextTool::selectFont()
{
    FontDia *fontDlg = new FontDia(m_textEditor.data());
    fontDlg->exec();
    delete fontDlg;
    returnFocusToCanvas();
}

void TextTool::shapeAddedToCanvas()
{
    kDebug();
    if (m_textShape) {
        KoSelection *selection = canvas()->shapeManager()->selection();
        KoShape *shape = selection->firstSelectedShape();
        if (shape != m_textShape && canvas()->shapeManager()->shapes().contains(m_textShape)) {
            // this situation applies when someone, not us, changed the selection by selecting another
            // text shape. Possibly by adding one.
            // Deselect the new shape again, so we can keep editing what we were already editing
            selection->select(m_textShape);
            selection->deselect(shape);
        }
    }
}

void TextTool::shapeDataRemoved()
{
    m_textShapeData = 0;
    m_textShape = 0;
    if (!m_textEditor.isNull() && !m_textEditor.data()->cursor()->isNull()) {
        const QTextDocument *doc = m_textEditor.data()->document();
        Q_ASSERT(doc);
        KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
        if (!lay || lay->shapes().isEmpty()) {
            emit done();
            return;
        }
        m_textShape = static_cast<TextShape*>(lay->shapes().first());
        m_textShapeData = static_cast<KoTextShapeData*>(m_textShape->userData());
        connect(m_textShapeData, SIGNAL(destroyed (QObject*)), this, SLOT(shapeDataRemoved()));
    }
}

void TextTool::createStyleFromCurrentBlockFormat(QString name)
{
    KoTextDocument document(m_textShapeData->document());
    KoStyleManager *styleManager = document.styleManager();
    KoParagraphStyle *paragraphStyle = new KoParagraphStyle(m_textEditor.data()->blockFormat(), m_textEditor.data()->charFormat());
    paragraphStyle->setName(name);
    styleManager->add(paragraphStyle);
    m_textEditor.data()->setStyle(paragraphStyle);
    emit charFormatChanged(m_textEditor.data()->charFormat(), m_textEditor.data()->blockCharFormat());
    emit blockFormatChanged(m_textEditor.data()->blockFormat());
}

void TextTool::createStyleFromCurrentCharFormat(QString name)
{
    KoTextDocument document(m_textShapeData->document());
    KoStyleManager *styleManager = document.styleManager();
    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(m_textEditor.data()->charFormat().intProperty(KoCharacterStyle::StyleId));
    KoCharacterStyle *autoStyle;
    if (!originalCharStyle) {
        KoCharacterStyle blankStyle;
        originalCharStyle = &blankStyle;
        autoStyle = originalCharStyle->autoStyle(m_textEditor.data()->charFormat(), m_textEditor.data()->blockCharFormat());
        autoStyle->setParentStyle(0);
    } else {
        autoStyle = originalCharStyle->autoStyle(m_textEditor.data()->charFormat(), m_textEditor.data()->blockCharFormat());
    }
    autoStyle->setName(name);
    styleManager->add(autoStyle);
    m_textEditor.data()->setStyle(autoStyle);
    emit charFormatChanged(m_textEditor.data()->charFormat(), m_textEditor.data()->blockCharFormat());
}

// ---------- editing plugins methods.
void TextTool::editingPluginEvents()
{
    if (m_prevCursorPosition == -1 || m_prevCursorPosition == m_textEditor.data()->position()) {
        kDebug()<<"m_prevCursorPosition="<<m_prevCursorPosition<<"m_textEditor.data()->position()="<<m_textEditor.data()->position();
        return;
    }

    QTextBlock block = m_textEditor.data()->block();
    if (! block.contains(m_prevCursorPosition)) {
        kDebug()<<"m_prevCursorPosition="<<m_prevCursorPosition;
        finishedWord();
        finishedParagraph();
        m_prevCursorPosition = -1;
    } else {
        int from = m_prevCursorPosition;
        int to = m_textEditor.data()->position();
        if (from > to)
            qSwap(from, to);
        QString section = block.text().mid(from - block.position(), to - from);
        kDebug()<<"from="<<from<<"to="<<to;
        if (section.contains(' ')) {
            finishedWord();
            m_prevCursorPosition = -1;
        }
    }
}

void TextTool::finishedWord()
{
    kDebug();
    if (m_textShapeData)
        foreach (KoTextEditingPlugin* plugin, m_textEditingPlugins->values())
            plugin->finishedWord(m_textShapeData->document(), m_prevCursorPosition);
}

void TextTool::finishedParagraph()
{
    kDebug();
    if (m_textShapeData)
        foreach (KoTextEditingPlugin* plugin, m_textEditingPlugins->values())
            plugin->finishedParagraph(m_textShapeData->document(), m_prevCursorPosition);
}

void TextTool::setTextColor(const KoColor &color)
{
    m_textEditor.data()->setTextColor(color.toQColor());
}

void TextTool::setBackgroundColor(const KoColor &color)
{
    m_textEditor.data()->setTextBackgroundColor(color.toQColor());
}

void TextTool::setGrowWidthToFit(bool enabled)
{
    m_textEditor.data()->addCommand(new AutoResizeCommand(m_textShapeData, KoTextShapeData::AutoGrowWidth, enabled));
    updateActions();
}

void TextTool::setGrowHeightToFit(bool enabled)
{
    m_textEditor.data()->addCommand(new AutoResizeCommand(m_textShapeData, KoTextShapeData::AutoGrowHeight, enabled));
    updateActions();
}

void TextTool::setShrinkToFit(bool enabled)
{
    m_textEditor.data()->addCommand(new AutoResizeCommand(m_textShapeData, KoTextShapeData::ShrinkToFitResize, enabled));
    updateActions();
}

void TextTool::runUrl(KoPointerEvent *event, QString &url)
{
    bool isLocalLink = (url.indexOf("file:") == 0);
    QString type = KMimeType::findByUrl(url, 0, isLocalLink)->name();

    if (KRun::isExecutableFile(url, type)) {
        QString question = i18n("This link points to the program or script '%1'.\n"
                                "Malicious programs can harm your computer. "
                                "Are you sure that you want to run this program?", url);
        // this will also start local programs, so adding a "don't warn again"
        // checkbox will probably be too dangerous
        int choice = KMessageBox::warningYesNo(0, question, i18n("Open Link?"));
        if (choice != KMessageBox::Yes)
            return;
    }

    event->accept();
    new KRun(url, 0);
}

void TextTool::debugTextDocument()
{
#ifndef NDEBUG
    if (!m_textShapeData)
        return;
    const int CHARSPERLINE = 80; // TODO Make configurable using ENV var?
    const int CHARPOSITION = 278301935;
    KoTextDocument document(m_textShapeData->document());
    KoStyleManager *styleManager = document.styleManager();
    KoInlineTextObjectManager *inlineManager = document.inlineTextObjectManager();

    QTextBlock block = m_textShapeData->document()->begin();
    for (;block.isValid(); block = block.next()) {
        QVariant var = block.blockFormat().property(KoParagraphStyle::StyleId);
        if (!var.isNull()) {
            KoParagraphStyle *ps = styleManager->paragraphStyle(var.toInt());
            kDebug(32500) << "--- Paragraph Style:" << (ps ? ps->name() : QString()) << var.toInt();
        }
        var = block.charFormat().property(KoCharacterStyle::StyleId);
        if (!var.isNull()) {
            KoCharacterStyle *cs = styleManager->characterStyle(var.toInt());
            kDebug(32500) << "--- Character Style:" << (cs ? cs->name() : QString()) << var.toInt();
        }
        int lastPrintedChar = -1;
        QTextBlock::iterator it;
        QString fragmentText;
        QList<QTextCharFormat> inlineCharacters;
        for (it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (!fragment.isValid())
                continue;
            QTextCharFormat fmt = fragment.charFormat();
            kDebug(32500) << "changeId: " << fmt.property(KoCharacterStyle::ChangeTrackerId);
            const int fragmentStart = fragment.position() - block.position();
            for (int i = fragmentStart; i < fragmentStart + fragment.length(); i += CHARSPERLINE) {
                if (lastPrintedChar == fragmentStart-1)
                    fragmentText += '|';
                if (lastPrintedChar < fragmentStart || i > fragmentStart) {
                    QString debug = block.text().mid(lastPrintedChar, CHARSPERLINE);
                    lastPrintedChar += CHARSPERLINE;
                    if (lastPrintedChar > block.length())
                        debug += "\\n";
                    kDebug(32500) << debug;
                }
                var = fmt.property(KoCharacterStyle::StyleId);
                QString charStyleLong, charStyleShort;
                if (! var.isNull()) { // named style
                    charStyleShort = QString::number(var.toInt());
                    KoCharacterStyle *cs = styleManager->characterStyle(var.toInt());
                    if (cs)
                        charStyleLong = cs->name();
                }
                if (inlineManager && fmt.hasProperty(KoCharacterStyle::InlineInstanceId)) {
                    QTextCharFormat inlineFmt = fmt;
                    inlineFmt.setProperty(CHARPOSITION, fragmentStart);
                    inlineCharacters << inlineFmt;
                }

                if (fragment.length() > charStyleLong.length())
                    fragmentText += charStyleLong;
                else if (fragment.length() > charStyleShort.length())
                    fragmentText += charStyleShort;
                else if (fragment.length() >= 2)
                    fragmentText += QChar(8230); // elipses



                int rest =  fragmentStart - (lastPrintedChar-CHARSPERLINE) + fragment.length() - fragmentText.length();
                rest = qMin(rest, CHARSPERLINE - fragmentText.length());
                if (rest >= 2)
                    fragmentText = QString("%1%2").arg(fragmentText).arg(' ', rest);
                if (rest >= 0)
                    fragmentText += '|';
                if (fragmentText.length() >= CHARSPERLINE) {
                    kDebug(32500) << fragmentText;
                    fragmentText.clear();
                }
            }
        }
        if (!fragmentText.isEmpty()) {
            kDebug(32500) << fragmentText;
        }
        else if (block.length() == 1) { // no actual tet
            kDebug(32500) << "\\n";
        }
        foreach (QTextCharFormat cf, inlineCharacters) {
            KoInlineObject *object= inlineManager->inlineTextObject(cf);
            kDebug(32500) << "At pos:" << cf.intProperty(CHARPOSITION) << object;
            // kDebug(32500) << "-> id:" << cf.intProperty(577297549);
        }
        QTextList *list = block.textList();
        if (list) {
            if (list->format().hasProperty(KoListStyle::StyleId)) {
                KoListStyle *ls = styleManager->listStyle(list->format().intProperty(KoListStyle::StyleId));
                kDebug(32500) << "   List style applied:" << ls->styleId() << ls->name();
            }
            else
                kDebug(32500) << " +- is a list..." << list;
        }
    }
#endif
}

void TextTool::debugTextStyles()
{
#ifndef NDEBUG
    if (!m_textShapeData)
        return;
    KoTextDocument document(m_textShapeData->document());
    KoStyleManager *styleManager = document.styleManager();

    QSet<int> seenStyles;

    foreach (KoParagraphStyle *style, styleManager->paragraphStyles()) {
        kDebug(32500) << style->styleId() << style->name() << (styleManager->defaultParagraphStyle() == style ? "[Default]" : "");
        KoListStyle *ls = style->listStyle();
        if (ls) { // optional ;)
            kDebug(32500) << "  +- ListStyle: " << ls->styleId() << ls->name()
                << (ls == styleManager->defaultListStyle() ? "[Default]":"");
            foreach (int level, ls->listLevels()) {
                KoListLevelProperties llp = ls->levelProperties(level);
                kDebug(32500) << "  |  level" << llp.level() << " style (enum):" << llp.style();
                if (llp.bulletCharacter().unicode() != 0) {
                    kDebug(32500) << "  |  bullet" << llp.bulletCharacter();
                }
            }
            seenStyles << ls->styleId();
        }
    }

    bool first = true;
    foreach (KoCharacterStyle *style, styleManager->characterStyles()) {
        if (seenStyles.contains(style->styleId()))
            continue;
        if (first) {
            kDebug(32500) << "--- Character styles ---";
            first = false;
        }
        kDebug(32500) << style->styleId() << style->name();
        kDebug(32500) << style->font();
    }

    first = true;
    foreach (KoListStyle *style, styleManager->listStyles()) {
        if (seenStyles.contains(style->styleId()))
            continue;
        if (first) {
            kDebug(32500) << "--- List styles ---";
            first = false;
        }
        kDebug(32500) << style->styleId() << style->name()
                << (style == styleManager->defaultListStyle() ? "[Default]":"");
    }
#endif
}

void TextTool::textDirectionChanged()
{
    QTextBlockFormat blockFormat;
    if (m_actionChangeDirection->isChecked()) {
        blockFormat.setProperty(KoParagraphStyle::TextProgressionDirection, KoText::RightLeftTopBottom);
    }
    else {
        blockFormat.setProperty(KoParagraphStyle::TextProgressionDirection, KoText::LeftRightTopBottom);
     }
    m_textEditor.data()->mergeBlockFormat(blockFormat);
}

void TextTool::setListLevel(int level)
{
    if (level < 1 || level > 10) {
        return;
    }

    KoTextEditor *textEditor = m_textEditor.data();
    if (textEditor->block().textList()) {
        ChangeListLevelCommand::CommandType type = ChangeListLevelCommand::SetLevel;
        ChangeListLevelCommand *cll = new ChangeListLevelCommand(*textEditor->cursor(), type, level);
        textEditor->addCommand(cll);
        editingPluginEvents();
    }
}

#include <TextTool.moc>
