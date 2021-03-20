// This file is part of PyKrita, Krita' Python scripting plugin.
//
// SPDX-FileCopyrightText: 2013 Alex Turbov <i.zaufi@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only

#ifndef __VERSION_CHECKER_H__
# define  __VERSION_CHECKER_H__

#include <libs/global/kis_debug.h>
#include "utilities.h"
# include <QtCore/QString>
# include <QtCore/QStringList>
# include <QtCore/QtGlobal>

namespace PyKrita
{

/**
 * \brief Class \c version
 */
class version
{
    enum type {
        undefined = -1,
        zero = 0
    };

public:
    /// Default constructor
    explicit version(const int _major = zero, const int _minor = zero, const int patch = zero)
        : m_major(_major)
        , m_minor(_minor)
        , m_patch(patch) {
    }

    int _major() const {
        return m_major;
    }
    int _minor() const {
        return m_minor;
    }
    int patch() const {
        return m_patch;
    }

    bool isValid() const {
        return _major() != undefined && _minor() != undefined && patch() != undefined;
    }

    operator QString() const {
        return QString("%1.%2.%3").arg(_major()).arg(_minor()).arg(patch());
    }

    static version fromString(const QString& version_str) {
        int tmp[3] = {zero, zero, zero};
        QStringList parts = version_str.split('.');
        for (
            unsigned long i = 0
                              ; i < qMin(static_cast<unsigned long>(sizeof(tmp) / sizeof(int)), static_cast<unsigned long>(parts.size()))
            ; ++i
        ) {
            bool ok;
            const int num = parts[i].toInt(&ok);
            if (ok)
                tmp[i] = num;
            else {
                tmp[i] = undefined;
                break;
            }
        }
        return version(tmp[0], tmp[1], tmp[2]);
    };

    static version fromPythonObject(PyObject* version_obj)
    {
        version v = tryObtainVersionFromTuple(version_obj);
        if (!v.isValid()) {
            // PEP396 requires __version__ to be a tuple of integers,
            // but some modules use a string instead.
            v = tryObtainVersionFromString(version_obj);
        }
        return v;
    }

    static version invalid() {
        static version s_bad(undefined, undefined, undefined);
        return s_bad;
    }

private:
    int m_major;
    int m_minor;
    int m_patch;


    static version tryObtainVersionFromTuple(PyObject* version_obj)
    {
        Q_ASSERT("Sanity check" && version_obj);

        if (PyTuple_Check(version_obj) == 0)
            return version::invalid();

        int version_info[3] = {0, 0, 0};
        for (unsigned i = 0; i < PyTuple_Size(version_obj); ++i) {
            PyObject* v = PyTuple_GetItem(version_obj, i);
            if (v && PyLong_Check(v))
                version_info[i] = PyLong_AsLong(v);
            else
                version_info[i] = -1;
        }
        if (version_info[0] != -1 && version_info[1] != -1 && version_info[2] != -1)
            return ::PyKrita::version(version_info[0], version_info[1], version_info[2]);

        return version::invalid();
    }

/**
 * Try to parse version string as a simple triplet X.Y.Z.
 *
 * \todo Some modules has letters in a version string...
 * For example current \c pytz version is \e "2013d".
 */
    static version tryObtainVersionFromString(PyObject* version_obj)
    {
        Q_ASSERT("Sanity check" && version_obj);

        if (!Python::isUnicode(version_obj))
            return version::invalid();

        QString version_str = Python::unicode(version_obj);
        if (version_str.isEmpty())
            return version::invalid();

        return version::fromString(version_str);
    }


};

inline bool operator==(const version& left, const version& right)
{
    return left._major() == right._major()
           && left._minor() == right._minor()
           && left.patch() == right.patch()
           ;
}

inline bool operator!=(const version& left, const version& right)
{
    return !(left == right);
}

inline bool operator<(const version& left, const version& right)
{
    return left._major() < right._major()
           || (left._major() == right._major() && left._minor() < right._minor())
           || (left._major() == right._major() && left._minor() == right._minor() && left.patch() < right.patch())
           ;
}

inline bool operator>(const version& left, const version& right)
{
    return left._major() > right._major()
           || (left._major() == right._major() && left._minor() > right._minor())
           || (left._major() == right._major() && left._minor() == right._minor() && left.patch() > right.patch())
           ;
}

inline bool operator<=(const version& left, const version& right)
{
    return left == right || left < right;
}

inline bool operator>=(const version& left, const version& right)
{
    return left == right || left > right;
}


/**
* \brief Class \c version_checker
*/
class version_checker
{
public:
    enum operation {
        invalid
        , undefined
        , less
        , less_or_equal
        , greater
        , greater_or_equal
        , not_equal
        , equal
        , last__
    };

    /// Default constructor
    explicit version_checker(const operation op = invalid)
        : m_op(op) {
    }

    bool isValid() const {
        return m_op != invalid;
    }

    bool isEmpty() const {
        return m_op == undefined;
    }

    void bind_second(const version& rhs) {
        m_rhs = rhs;
    }

    bool operator()(const version& left) {
        switch (m_op) {
        case less:
            return left < m_rhs;
        case greater:
            return left > m_rhs;
        case equal:
            return left == m_rhs;
        case not_equal:
            return left != m_rhs;
        case less_or_equal:
            return left <= m_rhs;
        case greater_or_equal:
            return left >= m_rhs;
        default:
            Q_ASSERT(!"Sanity check");
            break;
        }
        return false;
    }

    version required() const {
        return m_rhs;
    }

    QString operationToString() const {
        QString result;
        switch (m_op) {
        case less:
            result = " < ";
            break;
        case greater:
            result = " > ";
            break;
        case equal:
            result = " = ";
            break;
        case not_equal:
            result = " != ";
            break;
        case less_or_equal:
            result = " <= ";
            break;
        case greater_or_equal:
            result = " >= ";
            break;
        default:
            Q_ASSERT(!"Sanity check");
            break;
        }
        return result;
    }

    static version_checker fromString(const QString& version_info) {
        version_checker checker(invalid);
        if (version_info.isEmpty())
            return checker;

        bool lookup_next_char = false;
        int strip_lead_pos = 0;
        switch (version_info.at(0).toLatin1()) {
        case '<':
            checker.m_op = less;
            lookup_next_char = true;
            break;
        case '>':
            checker.m_op = greater;
            lookup_next_char = true;
            break;
        case '=':
            strip_lead_pos = 1;
            checker.m_op = equal;
            break;
        default:
            strip_lead_pos = 0;
            checker.m_op = equal;
            break;
        }
        if (lookup_next_char) {
            if (version_info.at(1).toLatin1() == '=') {
                // NOTE Shift state
                checker.m_op = operation(int(checker.m_op) + 1);
                strip_lead_pos = 2;
            } else {
                strip_lead_pos = 1;
            }
        }
        //
        QString rhs_str = version_info.mid(strip_lead_pos).trimmed();
        version rhs = version::fromString(rhs_str);
        if (rhs.isValid())
            checker.bind_second(rhs);
        else
            checker.m_op = invalid;
        return checker;
    }

private:
    operation m_op;
    version m_rhs;
};

}                                                           // namespace PyKrita
#endif                                                      //  __VERSION_CHECKER_H__
