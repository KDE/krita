#ifndef KISCHANGEVALUECOMMAND_H
#define KISCHANGEVALUECOMMAND_H

#include "kundo2command.h"

/**
 * A simple command that flips the state of a member variable in a
 * contained structure.
 *
 * For example, to change the variable `m_d->fooBar` to the value `newValue`
 * in an undoable fashion, just create a command like that:
 *
 *        \code{.cpp}
 *
 *        KUndo2Command *cmd =
 *            makeChangeValueCommand<&Private::fooBar>(m_d.data(), newValue);
 *
 *        \endcode
 *
 * It is not strictly necessary that `newValue` had the same type as the type
 * of the member variable. They may differ, given the types are convertible.
 */

template <auto mem_ptr, typename ValueType>
struct KisChangeValueCommand {};

template <typename Base, typename Member, Member Base::* mem_ptr, typename ValueType>
struct KisChangeValueCommand<mem_ptr, ValueType> : KUndo2Command
{
    KisChangeValueCommand(Base *base, const ValueType &newValue, KUndo2Command *parent = nullptr)
        : KUndo2Command(parent)
        , m_base(base)
        , m_oldValue(m_base->*mem_ptr)
        , m_newValue(newValue)
    {
    }

    void redo() {
        KUndo2Command::redo();
        m_base->*mem_ptr = m_newValue;
    }

    void undo() {
        m_base->*mem_ptr = m_oldValue;
        KUndo2Command::undo();
    }

protected:
    Base *m_base;
    ValueType m_oldValue;
    ValueType m_newValue;
};


/**
 * Creates KisChangeValueCommand on heap, automatically deducing the
 * type of `newValue`.
 */
template <auto mem_ptr, typename Base, typename ValueType>
KisChangeValueCommand<mem_ptr, ValueType> *makeChangeValueCommand(Base *base, const ValueType &newValue, KUndo2Command *parent = nullptr)
{
    return new KisChangeValueCommand<mem_ptr, ValueType>(base, newValue, parent);
}


/**
 * KisChangeIndirectValueCommand has the same purpose as KisChangeValueCommand, but
 * changes the value of variable stored **inside** a pointer. Basically, it
 * dereferences the variable on every assignment operation.
 */

template <auto mem_ptr, typename ValueType>
struct KisChangeIndirectValueCommand {};

template <typename Base, typename Member, Member Base::* mem_ptr, typename ValueType>
struct KisChangeIndirectValueCommand<mem_ptr, ValueType> : KUndo2Command
{
    KisChangeIndirectValueCommand(Base *base, const ValueType &newValue, KUndo2Command *parent = nullptr)
        : KUndo2Command(parent)
        , m_base(base)
        , m_oldValue(*(m_base->*mem_ptr))
        , m_newValue(newValue)
    {
    }

    void redo() {
        KUndo2Command::redo();
        *(m_base->*mem_ptr) = m_newValue;
    }

    void undo() {
        *(m_base->*mem_ptr) = m_oldValue;
        KUndo2Command::undo();
    }
private:
    Base *m_base;
    ValueType m_oldValue;
    ValueType m_newValue;
};

/**
 * Creates KisChangeIndirectValueCommand on heap, automatically deducing the
 * type of `newValue`.
 */
template <auto mem_ptr, typename Base, typename ValueType>
KisChangeIndirectValueCommand<mem_ptr, ValueType>* makeChangeIndirectValueCommand(Base *base, const ValueType &newValue, KUndo2Command *parent = nullptr)
{
    return new KisChangeIndirectValueCommand<mem_ptr, ValueType>(base, newValue, parent);
}

#endif // KISCHANGEVALUECOMMAND_H
