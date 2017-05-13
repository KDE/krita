/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef USERFEEDBACK_COMPILERINFOSOURCE_H
#define USERFEEDBACK_COMPILERINFOSOURCE_H

#include "abstractdatasource.h"

namespace UserFeedback {

/*! Data source reporting which compiler was used to build this code.
 *  @note This will report which compiler was used to build the feedback
 *  library, which technically does not need to be the same as used for
 *  the application code. This is particularly true for compilers that
 *  don't break ABI regularly (such as Clang and GCC), so this information
 *  is most reliable when this is not the case, e.g. with MSVC.
 */
class USERFEEDBACKCORE_EXPORT CompilerInfoSource : public AbstractDataSource
{
public:
    Q_DECLARE_TR_FUNCTIONS(UserFeedback::CompilerInfoSource)
public:
    CompilerInfoSource();
    QString description() const override;
    QVariant data() override;
};

}

#endif // USERFEEDBACK_COMPILERINFOSOURCE_H
