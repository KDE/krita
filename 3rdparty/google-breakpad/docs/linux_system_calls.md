# Introduction

Linux implements its userland-to-kernel transition using a special library
called linux-gate.so that is mapped by the kernel into every process. For more
information, see

http://www.trilithium.com/johan/2005/08/linux-gate/

In a nutshell, the problem is that the system call gate function,
kernel\_vsyscall does not use EBP to point to the frame pointer.

However, the Breakpad processor supports special frames like this via STACK
lines in the symbol file. If you look in src/client/linux/data you will see
symbol files for linux-gate.so for both Intel & AMD(the implementation of
kernel\_vsyscall changes depending on the CPU manufacturer). When processing
minidumps from Linux 2.6, having these symbol files is necessary for walking the
stack for crashes that happen while a thread is in a system call.

If you're just interested in processing minidumps, those two symbol files should
be all you need!

# Details

The particular details of understanding the linux-gate.so symbol files can be
found by reading about STACK lines inside
src/common/windows/pdb\_source\_line\_writer.cc, and the above link. To
summarize briefly, we just have to inform the processor how to get to the
previous frame when the EIP is inside kernel\_vsyscall, and we do that by
telling the processor how many bytes kernel\_vsyscall has pushed onto the stack
in it's prologue. For example, one of the symbol files looks somewhat like the
following:

MODULE Linux x86 random\_debug\_id linux-gate.so PUBLIC 400 0 kernel\_vsyscall
STACK WIN 4 100 1 1 0 0 0 0 0 1

The PUBLIC line indicates that kernel\_vsyscall is at offset 400 (in bytes) from
the beginning of linux-gate.so. The STACK line indicates the size of the
function(100), how many bytes it pushes(1), and how many bytes it pops(1). The
last 1 indicates that EBP is pushed onto the stack before being used by the
function.

# Warnings

These functions might change significantly depending on kernel version. In my
opinion, the actual function stack information is unlikely to change frequently,
but the Linux kernel might change the address of kernel\_vsyscall w.r.t the
beginning of linux-gate.so, which would cause these symbol files to be invalid.
