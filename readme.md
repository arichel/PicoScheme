PicoScheme
==========
A small, embeddable scheme interpreter in c++17. This project started as a
test bed to evaluate the new [std::variant] class template as plain c-union
replacement and to test shared, reference counting pointers for basic memory
menagement. Considering that there are already very feature complete, small
and efficient scheme implementations and to keep the implementation effort
reasonable, at least most scheme functions from the old [R4RS scheme] specification
are implemented. Postponed for now is a complete numeric tower, including rational
numbers and arbitrary precision arithmetic, and more advanced features like [call/cc]
and *define-syntax* macros, which I haven't fully understood so far. However
integer, floating point and complex numbers, call/cc as simple escape procedure
and old school lisp-style macros are implemented.

### Credits ###
While a more formal approach, on how to implement a dynamically typed,
functional language interpreter would definitely be advisable, I followed
here the ad hoc learning by doing approach and read the sources of some
publicly available scheme implementations first. Therefore I would like to
thank the following authors and maintainers for releasing their work:
- [minischeme] really miniature scheme by Atsushi Moriwaki and Cat's Eye Technologies.
- [TinyScheme] by Dimitrios Souflis, Kevin Cozens, Jonathan S. Shapiro and many others.
- [Jscheme] very readable Java implementation by Peter Norvig.
- [LispMe] interesting [SECD vm] scheme by Fred Bayer with [source code].
- [Chibi-Scheme] very feature complete [R7RS scheme] by Alex Shinn.

Installation and using the interpreter
--------------------------------------
The source code successfully compiles with a fairly recent *GNU g++* compiler
and under MS-Windows with the *Visual Studio Community Edition v15* c++ compiler
or the [MSYS2] GNU build tools.

### Compiling under Windows on the Command Line with Visual Studio build tools ###
Please make sure that either *Visual Studio with Desktop development with C++* or
the command-line *Build Tools for Visual Studio* is successfully installed.
Additionally please install the [Kitware CMake] build generator software.
- Open a developer-console for Visual Studio 2017 and change into your prefered
  working directory, for example `cd %home%`
- Download the PicoScheme sources and change into the unpacked source directory
  with `cd PicoScheme`
- or optional in case [Git version control] is installed,
  directly clone the PicoScheme github repository with
  `git clone https://github.com/arichel/PicoScheme.git` and change into
  the new PicoScheme source directoy.
- Create a new build sub directory with `mkdir build` an type:
  `cmake -H. -Bbuild` to generate a Visual Studio project file.
- Change into the build directory and type `msbuild PicoScheme.sln` to
  build the static library `picoscheme.lib` and test program `picoscm.exe`
- Optionally copy a scheme resource file from `cp ../test/picoscmrc.scm Debug/.`
  into the `Debug` or `Release` subdirectory.
- Change into the `Debug` subdirectory and type `picoscm.exe` to start
  an interactive scheme read-eval-print loop (repl). Type `(exit)` or `Ctrl+c`
  to quit the interpreter.

### Compiling under Windows with MSYS2 build tools ###
Please make sure that the [MSYS2] software building
platform is installed and a functional MSYS2 console is available.
- Open the msys2 console and optional update the package database with
  `pacman -Syu` then close the console, reopen and run again `pacman -Su`
- If you haven't already, please add the following packages:
  [Kitware CMake] with `pacman -S cmake`
- Optional if you like to clone the PicoScheme repository instead of
  just downloading the sources, additionally install the [Git version control]
  package with `pacman -S git`.

Now we are ready to clone or download and compile the PicoScheme sources:
- Change into your prefered installation directory and either unpack
  the downloaded PicoScheme.zip folder or if Git is installed, clone with:
  `git clone https://github.com/arichel/PicoScheme.git`
- Change into PicoScheme source directory and create a new build directory
  `mkdir build`
- Generate a unix makefile in this build directory with:
  `cmake -H. -Bbuild`
- Change into the `build` sub directory and type `make all` to build
  a static `libpicoscm.a` library and a `picoscm.exe` test program.
- Optional copy a scheme startup file `cp ../test/picoscmrc.scm` into the build
  directory and type `./picoscm.exe` to start the interpreter. Type `(exit)` or
  Ctrl+d to quit the interpreter.

Extending the interpreter
-------------------------
todo...

Implementation details
----------------------

### Scheme cells ###
A scheme cell is derived from a [std::variant] type, as declared in
[types.hpp](src/include/picoscm/types.hpp),
as a union of all supported types:
```c++
using Cons  = std::tuple<Cell, Cell, bool>;

using None  = std::monostate;
using Nil   = std::nullptr_t;
using Bool  = bool;
using Char  = wchar_t;

using StringPtr = std::shared_ptr<std::basic_string<Char>>;
using VectorPtr = std::shared_ptr<std::vector<Cell>>;
// ...

using Variant = std::variant <

    /* Atom types: */
    None, Nil, Intern, Bool, Char, Number,

    /* Compound value types: */
    Symbol, Procedure,

    /* Pointer types: */
    Cons*, StringPtr, VectorPtr, PortPtr, FunctionPtr, SymenvPtr
>;

struct Cell : Variant {
    using base_type = Variant;
    using Variant::Variant;
};
```
Derivation of structure *Cell* from *Variant* is necessary to forward declare a scheme
*Cons*-cell as a [std::tuple] of two *Cell* types itself. Atomic types, like booleans, characters,
numbers and symbols are directly stored as value types and compound types like strings,
vectors or IO-ports are stored as shared pointers. Symbols and procedures (closures) are
stored as a small handle class with internal pointer to the implementation class.
This assures that the byte size of a scheme cell remains reasonable small.
Currently, the largest value type are numbers with 16 bytes for a complex
number consisting of two double floating point values and additional eight bytes for
the variant type internals itself plus alignment padding. A scheme *Cell* structure has
therefore a size of 32 bytes in total (16 bytes + 8 bytes variant internals + 8 padding bytes).
Scheme *Cons*-cells are stored as plain c-pointers into the global cell store,
which is basically a [std::list] container of *Cons*-cell pairs.

### Garbage collector ###
todo...

MIT Licence
-----------
Copyright (c) 2018 Arichel

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


[minischeme]:   https://github.com/catseye/minischeme
[TinyScheme]:   http://tinyscheme.sourceforge.net/home.html
[Jscheme]:      https://norvig.com/jscheme.html
[LispMe]:       http://lispme.de/lispme/index_en.html
[Chibi-Scheme]: http://synthcode.com/scheme/chibi
[source code]:  https://github.com/arichel/LispMe

[SECD vm]:      https://en.wikipedia.org/wiki/SECD_machine
[call/cc]:      https://en.wikipedia.org/wiki/Call-with-current-continuation
[R4RS scheme]:  http://people.csail.mit.edu/jaffer/r4rs_toc.html
[R7RS scheme]:  http://r7rs.org

[std::variant]: https://en.cppreference.com/w/cpp/utility/variant
[std::list]:    https://en.cppreference.com/w/cpp/container/list
[std::tuple]:   https://en.cppreference.com/w/cpp/utility/tuple

[MSYS2]:               http://www.msys2.org/
[Kitware CMake]:       https://cmake.org/
[Git version control]: https://git-scm.com/

