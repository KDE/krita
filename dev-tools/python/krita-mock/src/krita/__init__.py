"""This is a mock krita module for Python unit tests.

This module returns a mock object for any attribute name and thus
prevents any errors surrounding the krita module in unit tests. This
makes it possible to write unit tests for Krita-independent code
units.

Caveats:

Will only work with proper imports:

 import krita
 krita.Krita.instance() # no-op on a mock object

Not with wildcard imports:

 from krita import *
 Krita.instance() # error

(Wildcard imports should be avoided anyway.)

"""

import builtins
import sys
from unittest.mock import MagicMock


sys.modules['krita'] = MagicMock()

builtins.i18n = lambda s: s
