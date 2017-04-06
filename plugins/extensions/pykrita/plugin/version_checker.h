// This file is part of PyKrita, Krita' Python scripting plugin.
//
// Copyright (C) 2013 Alex Turbov <i.zaufi@gmail.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) version 3.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; see the file COPYING.LIB.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#ifndef __VERSION_CHECKER_H__
# define  __VERSION_CHECKER_H__

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
        undefined = -1
                    , zero = 0
    };

public:
    /// Default constructor
    explicit version(
        const int major = zero
                          , const int minor = zero
                                  , const int patch = zero
    )
        : m_major(major)
        , m_minor(minor)
        , m_patch(patch) {
    }

    int major() const {
        return m_major;
    }
    int minor() const {
        return m_minor;
    }
    int patch() const {
        return m_patch;
    }

    bool isValid() const {
        return major() != undefined && minor() != undefined && patch() != undefined;
    }

    operator QString() const {
        return QString("%1.%2.%3").arg(major()).arg(minor()).arg(patch());
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

    static version invalid() {
        static version s_bad(undefined, undefined, undefined);
        return s_bad;
    }

private:
    int m_major;
    int m_minor;
    int m_patch;
};

inline bool operator==(const version& left, const version& right)
{
    return left.major() == right.major()
           && left.minor() == right.minor()
           && left.patch() == right.patch()
           ;
}

inline bool operator!=(const version& left, const version& right)
{
    return !(left == right);
}

inline bool operator<(const version& left, const version& right)
{
    return left.major() < right.major()
           || (left.major() == right.major() && left.minor() < right.minor())
           || (left.major() == right.major() && left.minor() == right.minor() && left.patch() < right.patch())
           ;
}

inline bool operator>(const version& left, const version& right)
{
    return left.major() > right.major()
           || (left.major() == right.major() && left.minor() > right.minor())
           || (left.major() == right.major() && left.minor() == right.minor() && left.patch() > right.patch())
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
        , greather
        , greather_or_equal
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
        case greather:
            return left > m_rhs;
        case equal:
            return left == m_rhs;
        case not_equal:
            return left != m_rhs;
        case less_or_equal:
            return left <= m_rhs;
        case greather_or_equal:
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
        case greather:
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
        case greather_or_equal:
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
        switch (version_info.at(0).toAscii()) {
        case '<':
            checker.m_op = less;
            lookup_next_char = true;
            break;
        case '>':
            checker.m_op = greather;
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
            if (version_info.at(1).toAscii() == '=') {
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
