"""
Copyright (c) 2017 Eliakin Costa <eliakim170@gmail.com>

SPDX-License-Identifier: GPL-2.0-or-later

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
"""
from PyQt5.QtWidgets import QWidget
from PyQt5.QtCore import QSize


class StatusBar(QWidget):

    def __init__(self, editor):
        super(StatusBar, self).__init__(editor)
        self.codeEditor = editor

    def sizeHint(self):
        return QSize(self.codeEditor.width(), 0)

    def paintEvent(self, event):
        """It Invokes the draw method(statusBarPaintEvent) in CodeEditor"""
        self.codeEditor.statusBarPaintEvent(event)
