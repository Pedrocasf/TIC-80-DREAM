// MIT License

// Copyright (c) 2017 Vadim Grigoruk @nesbox // grigoruk@gmail.com

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "vita.h"

#include <errno.h>
#include <psp2/power.h>

// newlib declares symlink() but the Vita has nothing to implement it with, its
// volumes are FAT and exFAT. vendor/zip calls it when a cartridge zip holds a
// symlink entry, extracting one just fails.
int symlink(const char* name1, const char* name2)
{
    errno = ENOSYS;
    return -1;
}

// The main thread runs the whole studio, the script parsers (moonscript, fennel,
// wren, ...) recurse quite deeply, so the 256KB default is not enough.
unsigned int sceUserMainThreadStackSize = 4 * 1024 * 1024;

// An application gets 256MB of user memory on a Vita, leave room for the
// stack, the code and everything SDL allocates outside of the newlib heap.
unsigned int _newlib_heap_size_user = 128 * 1024 * 1024;

void vita_init(void)
{
    // homebrew is launched with the shell clock profile, raise it to the
    // maximum a game gets, TIC-80 needs it to keep 60fps with the interpreters
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
}
