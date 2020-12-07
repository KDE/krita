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
from PyQt5.QtWidgets import QPlainTextEdit


class OutPutTextEdit(QPlainTextEdit):

    def __init__(self, scripter, toolbar, parent=None):
        super(OutPutTextEdit, self).__init__(parent)

        self.setObjectName('OutPutTextEdit')
        self.setReadOnly(True)
        doc = self.document()
        font = doc.defaultFont()
        font.setFamily('Monospace')
        doc.setDefaultFont(font)
