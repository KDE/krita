/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
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
 * #
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef KISMULTIFEEDRSSMODELTEST_H
#define KISMULTIFEEDRSSMODELTEST_H

#include <QObject>

class KisMultiFeedRssModelTest : public QObject
{
    Q_OBJECT
public:
    explicit KisMultiFeedRssModelTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testAddFeed();
    void testAddFeed_data();

    void testRemoveFeed();
    void testRemoveFeed_data();
};

#endif // KISMULTIFEEDRSSMODELTEST_H
