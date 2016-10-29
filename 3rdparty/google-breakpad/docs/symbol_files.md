# Introduction

Given a minidump file, the Breakpad processor produces stack traces that include
function names and source locations. However, minidump files contain only the
byte-by-byte contents of threads' registers and stacks, without function names
or machine-code-to-source mapping data. The processor consults Breakpad symbol
files for the information it needs to produce human-readable stack traces from
the binary-only minidump file.

The platform-specific symbol dumping tools parse the debugging information the
compiler provides (whether as DWARF or STABS sections in an ELF file or as
stand-alone PDB files), and write that information back out in the Breakpad
symbol file format. This format is much simpler and less detailed than compiler
debugging information, and values legibility over compactness.

# Overview

Breakpad symbol files are ASCII text files, with lines delimited as appropriate
for the host platform. Each line is a _record_, divided into fields by single
spaces; in some cases, the last field of the record can contain spaces. The
first field is a string indicating what sort of record the line represents
(except for line records; these are very common, making them the default saves
space). Some fields hold decimal or hexadecimal numbers; hexadecimal numbers
have no "0x" prefix, and use lower-case letters.

Breakpad symbol files contain the following record types. With some
restrictions, these may appear in any order.

*   A `MODULE` record describes the executable file or shared library from which
    this data was derived, for use by symbol suppliers. A `MODULE' record should
    be the first record in the file.

*   A `FILE` record gives a source file name, and assigns it a number by which
    other records can refer to it.

*   A `FUNC` record describes a function present in the source code.

*   A line record indicates to which source file and line a given range of
    machine code should be attributed. The line is attributed to the function
    defined by the most recent `FUNC` record.

*   A `PUBLIC` record gives the address of a linker symbol.

*   A `STACK` record provides information necessary to produce stack traces.

# `MODULE` records

A `MODULE` record provides meta-information about the module the symbol file
describes. It has the form:

> `MODULE` _operatingsystem_ _architecture_ _id_ _name_

For example: `MODULE Linux x86 D3096ED481217FD4C16B29CD9BC208BA0 firefox-bin
` These records provide meta-information about the executable or shared library
from which this symbol file was generated. A symbol supplier might use this
information to find the correct symbol files to use to interpret a given
minidump, or to perform other sorts of validation. If present, a `MODULE` record
should be the first line in the file.

The fields are separated by spaces, and cannot contain spaces themselves, except
for _name_.

*   The _operatingsystem_ field names the operating system on which the
    executable or shared library was intended to run. This field should have one
    of the following values: | **Value** | **Meaning** |
    |:----------|:--------------------| | Linux | Linux | | mac | Macintosh OSX
    | | windows | Microsoft Windows |

*   The _architecture_ field indicates what processor architecture the
    executable or shared library contains machine code for. This field should
    have one of the following values: | **Value** | **Instruction Set
    Architecture** | |:----------|:---------------------------------| | x86 |
    Intel IA-32 | | x86\_64 | AMD64/Intel 64 | | ppc | 32-bit PowerPC | | ppc64
    | 64-bit PowerPC | | unknown | unknown |

*   The _id_ field is a sequence of hexadecimal digits that identifies the exact
    executable or library whose contents the symbol file describes. The way in
    which it is computed varies from platform to platform.

*   The _name_ field contains the base name (the final component of the
    directory path) of the executable or library. It may contain spaces, and
    extends to the end of the line.

# `FILE` records

A `FILE` record holds a source file name for other records to refer to. It has
the form:

> `FILE` _number_ _name_

For example: `FILE 2 /home/jimb/mc/in/browser/app/nsBrowserApp.cpp
`

A `FILE` record provides the name of a source file, and assigns it a number
which other records (line records, in particular) can use to refer to that file
name. The _number_ field is a decimal number. The _name_ field is the name of
the file; it may contain spaces.

# `FUNC` records

A `FUNC` record describes a source-language function. It has the form:

> `FUNC` _address_ _size_ _parameter\_size_ _name_

For example: `FUNC c184 30 0 nsQueryInterfaceWithError::operator()(nsID const&,
void**) const
`

The _address_ and _size_ fields are hexadecimal numbers indicating the start
address and length in bytes of the machine code instructions the function
occupies. (Breakpad symbol files cannot accurately describe functions whose code
is not contiguous.) The start address is relative to the module's load address.

The _parameter\_size_ field is a hexadecimal number indicating the size, in
bytes, of the arguments pushed on the stack for this function. Some calling
conventions, like the Microsoft Windows `stdcall` convention, require the called
function to pop parameters passed to it on the stack from its caller before
returning. The stack walker uses this value, along with data from `STACK`
records, to step from the called function's frame to the caller's frame.

The _name_ field is the name of the function. In languages that use linker
symbol name mangling like C++, this should be the source language name (the
"unmangled" form). This field may contain spaces.

# Line records

A line record describes the source file and line number to which a given range
of machine code should be attributed. It has the form:

> _address_ _size_ _line_ _filenum_

For example: `c184 7 59 4
`

Because they are so common, line records do not begin with a string indicating
the record type. All other record types' names use upper-case letters;
hexadecimal numbers, like a line record's _address_, use lower-case letters.

The _address_ and _size_ fields are hexadecimal numbers indicating the start
address and length in bytes of the machine code. The address is relative to the
module's load address.

The _line_ field is the line number to which the machine code should be
attributed, in decimal; the first line of the source file is line number 1. The
_filenum_ field is a decimal number appearing in a prior `FILE` record; the name
given in that record is the source file name for the machine code.

The line is assumed to belong to the function described by the last preceding
`FUNC` record. Line records may not appear before the first `FUNC' record.

No two line records in a symbol file cover the same range of addresses. However,
there may be many line records with identical line and file numbers, as a given
source line may contribute many non-contiguous blocks of machine code.

# `PUBLIC` records

A `PUBLIC` record describes a publicly visible linker symbol, such as that used
to identify an assembly language entry point or region of memory. It has the
form:

> PUBLIC _address_ _parameter\_size_ _name_

For example: `PUBLIC 2160 0 Public2_1
`

The Breakpad processor essentially treats a `PUBLIC` record as defining a
function with no line number data and an indeterminate size: the code extends to
the next address mentioned. If a given address is covered by both a `PUBLIC`
record and a `FUNC` record, the processor uses the `FUNC` data.

The _address_ field is a hexadecimal number indicating the symbol's address,
relative to the module's load address.

The _parameter\_size_ field is a hexadecimal number indicating the size of the
parameters passed to the code whose entry point the symbol marks, if known. This
field has the same meaning as the _parameter\_size_ field of a `FUNC` record;
see that description for more details.

The _name_ field is the name of the symbol. In languages that use linker symbol
name mangling like C++, this should be the source language name (the "unmangled"
form). This field may contain spaces.

# `STACK WIN` records

Given a stack frame, a `STACK WIN` record indicates how to find the frame that
called it. It has the form:

> STACK WIN _type_ _rva_ _code\_size_ _prologue\_size_ _epilogue\_size_
> _parameter\_size_ _saved\_register\_size_ _local\_size_ _max\_stack\_size_
> _has\_program\_string_ _program\_string\_OR\_allocates\_base\_pointer_

For example: `STACK WIN 4 2170 14 1 0 0 0 0 0 1 $eip 4 + ^ = $esp $ebp 8 + =
$ebp $ebp ^ =
`

All fields of a `STACK WIN` record, except for the last, are hexadecimal
numbers.

The _type_ field indicates what sort of stack frame data this record holds. Its
value should be one of the values of the [StackFrameTypeEnum]
(http://msdn.microsoft.com/en-us/library/bc5207xw%28VS.100%29.aspx) type in
Microsoft's [Debug Interface Access (DIA)]
(http://msdn.microsoft.com/en-us/library/x93ctkx8%28VS.100%29.aspx) API.
Breakpad uses only records of type 4 (`FrameTypeFrameData`) and 0
(`FrameTypeFPO`); it ignores others. These types differ only in whether the last
field is an _allocates\_base\_pointer_ flag (`FrameTypeFPO`) or a program string
(`FrameTypeFrameData`). If more than one record covers a given address, Breakpad
prefers `FrameTypeFrameData` records over `FrameTypeFPO` records.

The _rva_ and _code\_size_ fields give the starting address and length in bytes
of the machine code covered by this record. The starting address is relative to
the module's load address.

The _prologue\_size_ and _epilogue\_size_ fields give the length, in bytes, of
the prologue and epilogue machine code within the record's range. Breakpad does
not use these values.

The _parameter\_size_ field gives the number of argument bytes this function
expects to have been passed. This field has the same meaning as the
_parameter\_size_ field of a `FUNC` record; see that description for more
details.

The _saved\_register\_size_ field gives the number of bytes in the stack frame
dedicated to preserving the values of any callee-saves registers used by this
function.

The _local\_size_ field gives the number of bytes in the stack frame dedicated
to holding the function's local variables and temporary values.

The _max\_stack\_size_ field gives the maximum number of bytes pushed on the
stack in the frame. Breakpad does not use this value.

If the _has\_program\_string_ field is zero, then the `STACK WIN` record's final
field is an _allocates\_base\_pointer_ flag, as a hexadecimal number; this is
expected for records whose _type_ is 0. Otherwise, the final field is a program
string.

## Interpreting a `STACK WIN` record

Given the register values for a frame F, we can find the calling frame as
follows:

*   If the _has\_program\_string_ field of a `STACK WIN` record is zero, then
    the final field is _allocates\_base\_pointer_, a flag indicating whether the
    frame uses the frame pointer register, `%ebp`, as a general-purpose
    register.
    *   If _allocates\_base\_pointer_ is true, then `%ebp` does not point to the
        frame's base address. Instead,
        *   Let _next\_parameter\_size_ be the parameter size of the function
            frame F called (**not** this record's _parameter\_size_ field), or
            zero if F is the youngest frame on the stack. You must find this
            value in F's callee's `FUNC`, `STACK WIN`, or `PUBLIC` records.
        *   Let _frame\_size_ be the sum of the _local\_size_ field, the
            _saved\_register\_size_ field, and _next\_parameter\_size_. > > With
            those definitions in place, we can recover the calling frame as
            follows:
        *   F's return address is at `%esp +`_frame\_size_,
        *   the caller's value of `%ebp` is saved at `%esp
            +`_next\_parameter\_size_`+`_saved\_register\_size_`- 8`, and
        *   the caller's value of `%esp` just before the call instruction was
            `%esp +`_frame\_size_`+ 4`. > > (Why do we include
            _next\_parameter\_size_ in the sum when computing _frame\_size_ and
            the address of the saved `%ebp`? When a function A has called a
            function B, the arguments that A pushed for B are considered part of
            A's stack frame: A's value for `%esp` points at the last argument
            pushed for B. Thus, we must include the size of those arguments
            (given by the debugging info for B) along with the size of A's
            register save area and local variable area (given by the debugging
            info for A) when computing the overall size of A's frame.)
    *   If _allocates\_base\_pointer_ is false, then F's function doesn't use
        `%ebp` at all. You may recover the calling frame as above, except that
        the caller's value of `%ebp` is the same as F's value for `%ebp`, so no
        steps are necessary to recover it.
*   If the _has\_program\_string_ field of a `STACK WIN` record is not zero,
    then the record's final field is a string containing a program to be
    interpreted to recover the caller's frame. The comments in the
    [postfix\_evaluator.h]
    (http://code.google.com/p/google-breakpad/source/browse/trunk/src/processor/postfix_evaluator.h#40)
    header file explain the language in which the program is written. You should
    place the following variables in the dictionary before interpreting the
    program:
    *   `$ebp` and `$esp` should be the values of the `%ebp` and `%esp`
        registers in F.
    *   `.cbParams`, `.cbSavedRegs`, and `.cbLocals`, should be the values of
        the `STACK WIN` record's _parameter\_size_, _saved\_register\_size_, and
        _local\_size_ fields.
    *   `.raSearchStart` should be set to the address on the stack to begin
        scanning for a return address, if necessary. The Breakpad processor sets
        this to the value of `%esp` in F, plus the _frame\_size_ value mentioned
        above.

> If the program stores values for `$eip`, `$esp`, `$ebp`, `$ebx`, `$esi`, or
> `$edi`, then those are the values of the given registers in the caller. If the
> value of `$eip` is zero, that indicates that the end of the stack has been
> reached.

The Breakpad processor checks that the value yielded by the above for the
calling frame's instruction address refers to known code; if the address seems
to be bogus, then it uses a heuristic search to find F's return address and
stack base.

# `STACK CFI` records

`STACK CFI` ("Call Frame Information") records describe how to walk the stack
when execution is at a given machine instruction. These records take one of two
forms:

> `STACK CFI INIT` _address_ _size_ _register<sub>1</sub>_:
> _expression<sub>1</sub>_ _register<sub>2</sub>_: _expression<sub>2</sub>_ ...
>
> `STACK CFI` _address_ _register<sub>1</sub>_: _expression<sub>1</sub>_
> _register<sub>2</sub>_: _expression<sub>2</sub>_ ...

For example:

```
STACK CFI INIT 804c4b0 40 .cfa: $esp 4 + $eip: .cfa 4 - ^
STACK CFI 804c4b1 .cfa: $esp 8 + $ebp: .cfa 8 - ^
```

The _address_ and _size_ fields are hexadecimal numbers. Each
_register_<sub>i</sub> is the name of a register or pseudoregister. Each
_expression_ is a Breakpad postfix expression, which may contain spaces, but
never ends with a colon. (The appropriate register names for a given
architecture are determined when `STACK CFI` records are first enabled for that
architecture, and should be documented in the appropriate
`stackwalker_`_architecture_`.cc` source file.)

STACK CFI records describe, at each machine instruction in a given function, how
to recover the values the machine registers had in the function's caller.
Naturally, some registers' values are simply lost, but there are three cases in
which they can be recovered:

*   You can always recover the program counter, because that's the function's
    return address. If the function is ever going to return, the PC must be
    saved somewhere.

*   You can always recover the stack pointer. The function is responsible for
    popping its stack frame before it returns to the caller, so it must be able
    to restore this, as well.

*   You should be able to recover the values of callee-saves registers. These
    are registers whose values the callee must preserve, either by saving them
    in its own stack frame before using them and re-loading them before
    returning, or by not using them at all.

(As an exception, note that functions which never return may not save any of
this data. It may not be possible to walk the stack past such functions' stack
frames.)

Given rules for recovering the values of a function's caller's registers, we can
walk up the stack. Starting with the current set of registers --- the PC of the
instruction we're currently executing, the current stack pointer, etc. --- we
use CFI to recover the values those registers had in the caller of the current
frame. This gives us a PC in the caller whose CFI we can look up; we apply the
process again to find that function's caller; and so on.

Concretely, CFI records represent a table with a row for each machine
instruction address and a column for each register. The table entry for a given
address and register contains a rule describing how, when the PC is at that
address, to restore the value that register had in the caller.

There are some special columns:

*   A column named `.cfa`, for "Canonical Frame Address", tells how to compute
    the base address of the frame; other entries can refer to the CFA in their
    rules.

*   A column named `.ra` represents the return address.

For example, suppose we have a machine with 32-bit registers, one-byte
instructions, a stack that grows downwards, and an assembly language that
resembles C. Suppose further that we have a function whose machine code looks
like this:

```
func:                                ; entry point; return address at sp
func+0:      sp -= 16                ; allocate space for stack frame
func+1:      sp[12] = r0             ; save 4-byte r0 at sp+12
             ...                     ; stuff that doesn't affect stack
func+10:     sp -= 4; *sp = x        ; push some 4-byte x on the stack
             ...                     ; stuff that doesn't affect stack
func+20:     r0 = sp[16]             ; restore saved r0
func+21:     sp += 20                ; pop whole stack frame
func+22:     pc = *sp; sp += 4       ; pop return address and jump to it
```

The following table would describe the function above:

**code address** | **.cfa** | **r0 (on Google Code)** | **r1 (on Google Code)** | ... | **.ra**
:--------------- | :------- | :---------------------- | :---------------------- | :-- | :-------
func+0           | sp       |                         |                         |     | `cfa[0]`
func+1           | sp+16    |                         |                         |     | `cfa[0]`
func+2           | sp+16    | `cfa[-4]`               |                         |     | `cfa[0]`
func+11          | sp+20    | `cfa[-4]`               |                         |     | `cfa[0]`
func+21          | sp+20    |                         |                         |     | `cfa[0]`
func+22          | sp       |                         |                         |     | `cfa[0]`

Some things to note here:

*   Each row describes the state of affairs **before** executing the instruction
    at the given address. Thus, the row for func+0 describes the state before we
    execute the first instruction, which allocates the stack frame. In the next
    row, the formula for computing the CFA has changed, reflecting the
    allocation.

*   The other entries are written in terms of the CFA; this allows them to
    remain unchanged as the stack pointer gets bumped around. For example, to
    find the caller's value for r0 (on Google Code) at func+2, we would first
    compute the CFA by adding 16 to the sp, and then subtract four from that to
    find the address at which r0 (on Google Code) was saved.

*   Although the example doesn't show this, most calling conventions designate
    "callee-saves" and "caller-saves" registers. The callee must restore the
    values of "callee-saves" registers before returning (if it uses them at
    all), whereas the callee is free to use "caller-saves" registers without
    restoring their values. A function that uses caller-saves registers
    typically does not save their original values at all; in this case, the CFI
    marks such registers' values as "unrecoverable".

*   Exactly where the CFA points in the frame --- at the return address? below
    it? At some fixed point within the frame? --- is a question of definition
    that depends on the architecture and ABI in use. But by definition, the CFA
    remains constant throughout the lifetime of the frame. It's up to
    architecture- specific code to know what significance to assign the CFA, if
    any.

To save space, the most common type of CFI record only mentions the table
entries at which changes take place. So for the above, the CFI data would only
actually mention the non-blank entries here:

**insn** | **cfa** | **r0 (on Google Code)** | **r1 (on Google Code)** | ... | **ra**
:------- | :------ | :---------------------- | :---------------------- | :-- | :-------
func+0   | sp      |                         |                         |     | `cfa[0]`
func+1   | sp+16   |                         |                         |     |
func+2   |         | `cfa[-4]`               |                         |     |
func+11  | sp+20   |                         |                         |     |
func+21  |         | r0 (on Google Code)     |                         |     |
func+22  | sp      |                         |                         |     |

A `STACK CFI INIT` record indicates that, at the machine instruction at
_address_, belonging to some function, the value that _register<sub>n</sub>_ had
in that function's caller can be recovered by evaluating
_expression<sub>n</sub>_. The values of any callee-saves registers not mentioned
are assumed to be unchanged. (`STACK CFI` records never mention caller-saves
registers.) These rules apply starting at _address_ and continue up to, but not
including, the address given in the next `STACK CFI` record. The _size_ field is
the total number of bytes of machine code covered by this record and any
subsequent `STACK CFI` records (until the next `STACK CFI INIT` record). The
_address_ field is relative to the module's load address.

A `STACK CFI` record (no `INIT`) is the same, except that it mentions only those
registers whose recovery rules have changed from the previous CFI record. There
must be a prior `STACK CFI INIT` or `STACK CFI` record in the symbol file. The
_address_ field of this record must be greater than that of the previous record,
and it must not be at or beyond the end of the range given by the most recent
`STACK CFI INIT` record. The address is relative to the module's load address.

Each expression is a breakpad-style postfix expression. Expressions may contain
spaces, but their tokens may not end with colons. When an expression mentions a
register, it refers to the value of that register in the callee, even if a prior
name/expression pair gives that register's value in the caller. The exception is
`.cfa`, which refers to the canonical frame address computed by the .cfa rule in
force at the current instruction.

The special expression `.undef` indicates that the given register's value cannot
be recovered.

The register names preceding the expressions are always followed by colons. The
expressions themselves never contain tokens ending with colons.

There are two special register names:

*   `.cfa` ("Canonical Frame Address") is the base address of the stack frame.
    Other registers' rules may refer to this. If no rule is provided for the
    stack pointer, the value of `.cfa` is the caller's stack pointer.

*   `.ra` is the return address. This is the value of the restored program
    counter. We use `.ra` instead of the architecture-specific name for the
    program counter.

The Breakpad stack walker requires that there be rules in force for `.cfa` and
`.ra` at every code address from which it unwinds. If those rules are not
present, the stack walker will ignore the `STACK CFI` data, and try to use a
different strategy.

So the CFI for the example function above would be as follows, if `func` were at
address 0x1000 (relative to the module's load address):

```
STACK CFI INIT 1000 .cfa: $sp .ra: .cfa ^
STACK CFI      1001 .cfa: $sp 16 +
STACK CFI      1002 $r0: .cfa 4 - ^
STACK CFI      100b .cfa: $sp 20 +
STACK CFI      1015 $r0: $r0
STACK CFI      1016 .cfa: $sp
```
