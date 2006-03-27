/**
 * @libdoc A formula editor library.
 *
 * The kformula editor library. There are three classes you might be interested in.
 * First there is the @ref Document . It is, you guess it, the document.
 * Next there is the @ref Container . It represents one formula. Each
 * formula belongs to a document and a document might have any number of formulas.
 * For this reason the formula should be considered as part of the document.
 * The last class is
 * @ref KFormulaWidget . This one is a widget that serves as view.
 * You can create any number of views for each formula.
 *
 * The document contains functions like load/save and the undo stack as well as
 * everything you need to edit the formula.
 * The view provides the interface for cursor movement. Additional there is basic
 * key handling (for keys that probably shouldn't be actions). However every function
 * is a slot so you can use actions as well.
 * 
 * That's all you need to know to use it. But anyway:
 * At its heart this library contains formula elements. Each element is a child
 * of @ref BasicElement . The elements are meant to be selfcontained and thus
 * extendable.
 * A formula consists of a tree of those elements.
 *
 * To be able to work on the element tree in a generic way there is a
 * @ref FormulaCursor . Every edit operation goes through it. This way
 * the cursor separetes the element tree from the outside world.
 *
 * If you want to edit the formula you ask the document (@ref Container )
 * to create and execute a @ref Command . The commands use the currently
 * active cursor to do their editing. Each command can be undone.
 */
