/*
* SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
* SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
* SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCommandLineParser>

#include <kritaversion.h>
#include <kritagitversion.h>

QTextStream& qStdOut()
{
    static QTextStream ts( stdout );
    return ts;
}

extern "C" int main(int argc, char **argv)
{
    QStringList argList;
    for (int i = 0; i < argc; i++) {
        argList << QString::fromLocal8Bit(argv[i]);
    }

    QCommandLineParser parser;
    QCommandLineOption verbose(QStringList() << "v" << "verbose" << "showhash");
    parser.addOption(verbose);
    parser.process(argList);

    qStdOut() << KRITA_VERSION_STRING;
#ifdef KRITA_GIT_SHA1_STRING
    if (parser.isSet(verbose)) {
        qStdOut() << " " << KRITA_GIT_SHA1_STRING;
    }
#endif
    qStdOut() << "\n";
    return 0;
}
