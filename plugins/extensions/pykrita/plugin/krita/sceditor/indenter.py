#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import re

from rope.base import codeanalyze


class TextIndenter(object):

    """A class for formatting texts"""

    def __init__(self, editor, indents=4):
        self.editor = editor
        self.indents = indents
        self.line_editor = editor.line_editor()

    def correct_indentation(self, lineno):
        """Correct the indentation of a line"""

    def deindent(self, lineno):
        """Deindent the a line"""
        current_indents = self._count_line_indents(lineno)
        new_indents = max(0, current_indents - self.indents)
        self._set_line_indents(lineno, new_indents)

    def indent(self, lineno):
        """Indent a line"""
        current_indents = self._count_line_indents(lineno)
        new_indents = current_indents + self.indents
        self._set_line_indents(lineno, new_indents)

    def entering_new_line(self, lineno):
        """Indent a line

        Uses `correct_indentation` and last line indents
        """
        last_line = ""
        if lineno > 1:
            last_line = self.line_editor.get_line(lineno - 1)
        if last_line.strip() == '':
            self._set_line_indents(lineno, len(last_line))
        else:
            self.correct_indentation(lineno)

    def insert_tab(self, index):
        """Inserts a tab in the given index"""
        self.editor.insert(index, ' ' * self.indents)

    def _set_line_indents(self, lineno, indents):
        old_indents = self._count_line_indents(lineno)
        indent_diffs = indents - old_indents
        self.line_editor.indent_line(lineno, indent_diffs)

    def _count_line_indents(self, lineno):
        contents = self.line_editor.get_line(lineno)
        result = 0
        for x in contents:
            if x == ' ':
                result += 1
            elif x == '\t':
                result += 8
            else:
                break
        return result


class NormalIndenter(TextIndenter):

    def __init__(self, editor):
        super(NormalIndenter, self).__init__(editor)

    def correct_indentation(self, lineno):
        prev_indents = 0
        if lineno > 1:
            prev_indents = self._count_line_indents(lineno - 1)
        self._set_line_indents(lineno, prev_indents)


class PythonCodeIndenter(TextIndenter):

    def __init__(self, editor, indents=4):
        super(PythonCodeIndenter, self).__init__(editor, indents)

    def _last_non_blank(self, lineno):
        current_line = lineno - 1
        while current_line != 1 and \
                self.line_editor.get_line(current_line).strip() == '':
            current_line -= 1
        return current_line

    def _get_correct_indentation(self, lineno):
        if lineno == 1:
            return 0
        new_indent = self._get_base_indentation(lineno)

        prev_lineno = self._last_non_blank(lineno)
        prev_line = self.line_editor.get_line(prev_lineno)
        if prev_lineno == lineno or prev_line.strip() == '':
            new_indent = 0
        current_line = self.line_editor.get_line(lineno)
        new_indent += self._indents_caused_by_current_stmt(current_line)
        return new_indent

    def _get_base_indentation(self, lineno):
        range_finder = _StatementRangeFinder(
            self.line_editor, self._last_non_blank(lineno))
        start = range_finder.get_statement_start()
        if not range_finder.is_line_continued():
            changes = self._indents_caused_by_prev_stmt(
                (start, self._last_non_blank(lineno)))
            return self._count_line_indents(start) + changes
        if range_finder.last_open_parens():
            open_parens = range_finder.last_open_parens()
            parens_line = self.line_editor.get_line(open_parens[0])
            if parens_line[open_parens[1] + 1:].strip() == '':
                if len(range_finder.open_parens) > 1:
                    return range_finder.open_parens[-2][1] + 1
                else:
                    return self._count_line_indents(start) + self.indents
            return range_finder.last_open_parens()[1] + 1

        start_line = self.line_editor.get_line(start)
        if start == lineno - 1:
            try:
                equals_index = start_line.index(' = ') + 1
                if start_line[equals_index + 1:].strip() == '\\':
                    return self._count_line_indents(start) + self.indents
                return equals_index + 2
            except ValueError:
                match = re.search(r'(\b )|(\.)', start_line)
                if match:
                    return match.start() + 1
                else:
                    return len(start_line) + 1
        else:
            return self._count_line_indents(self._last_non_blank(lineno))

    def _indents_caused_by_prev_stmt(self, stmt_range):
        first_line = self.line_editor.get_line(stmt_range[0])
        last_line = self.line_editor.get_line(stmt_range[1])
        new_indent = 0
        if self._strip(last_line).endswith(':'):
            new_indent += self.indents
        if self._startswith(first_line, ('return', 'raise', 'pass',
                                         'break', 'continue')):
            new_indent -= self.indents
        return new_indent

    def _startswith(self, line, tokens):
        line = self._strip(line)
        for token in tokens:
            if line == token or line.startswith(token + ' '):
                return True

    def _strip(self, line):
        try:
            numsign = line.rindex('#')
            comment = line[numsign:]
            if '\'' not in comment and '\"' not in comment:
                line = line[:numsign]
        except ValueError:
            pass
        return line.strip()

    def _indents_caused_by_current_stmt(self, current_line):
        new_indent = 0
        if self._strip(current_line) == 'else:':
            new_indent -= self.indents
        if self._strip(current_line) == 'finally:':
            new_indent -= self.indents
        if self._startswith(current_line, ('elif',)):
            new_indent -= self.indents
        if self._startswith(current_line, ('except',)) and \
           self._strip(current_line).endswith(':'):
            new_indent -= self.indents
        return new_indent

    def correct_indentation(self, lineno):
        """Correct the indentation of the line containing the given index"""
        self._set_line_indents(lineno, self._get_correct_indentation(lineno))


class _StatementRangeFinder(object):

    """A method object for finding the range of a statement"""

    def __init__(self, lines, lineno):
        self.lines = lines
        self.lineno = lineno
        self.in_string = ''
        self.open_count = 0
        self.explicit_continuation = False
        self.open_parens = []
        self._analyze()

    def _analyze_line(self, lineno):
        current_line = self.lines.get_line(lineno)
        for i, char in enumerate(current_line):
            if char in '\'"':
                if self.in_string == '':
                    self.in_string = char
                    if char * 3 == current_line[i:i + 3]:
                        self.in_string = char * 3
                elif self.in_string == current_line[i:i + len(self.in_string)] and \
                    not (i > 0 and current_line[i - 1] == '\\' and
                         not (i > 1 and current_line[i - 2:i] == '\\\\')):
                    self.in_string = ''
            if self.in_string != '':
                continue
            if char == '#':
                break
            if char in '([{':
                self.open_count += 1
                self.open_parens.append((lineno, i))
            if char in ')]}':
                self.open_count -= 1
                if self.open_parens:
                    self.open_parens.pop()
        if current_line and char != '#' and current_line.endswith('\\'):
            self.explicit_continuation = True
        else:
            self.explicit_continuation = False

    def _analyze(self):
        last_statement = 1
        block_start = codeanalyze.get_block_start(self.lines, self.lineno)
        for current_line_number in range(block_start, self.lineno + 1):
            if not self.explicit_continuation and \
               self.open_count == 0 and self.in_string == '':
                last_statement = current_line_number
            self._analyze_line(current_line_number)
        self.statement_start = last_statement

    def get_statement_start(self):
        return self.statement_start

    def last_open_parens(self):
        if not self.open_parens:
            return None
        return self.open_parens[-1]

    def is_line_continued(self):
        return self.open_count != 0 or self.explicit_continuation

    def get_line_indents(self, line_number):
        return self._count_line_indents(self.lines.get_line(line_number))
