PicoScheme
==========

A small embeddable scheme interpreter implementation in c++17. This project was
started because I wanted to learn recent c++ features, especially to evaluate the
new [std::variant] template class as plain c-union replacement and shared pointers
for basic memory management. Considering that there are already very feature
complete, embeddable and efficient small scheme implementations, and to keep
the implementation effort reasonable, at least most scheme functions from the
old [R4RS scheme] specification are implemented. Postponed for now is the
implementation of a complete numeric tower, including rational numbers and
arbitrary precision arithmetic, and more advanced features like call/cc and
define-syntax macros. So far integer, floating point and complex
numbers and old school lisp-style macros are implemented.

### Credits ###
While a more formal approach, on how to implement a dynamically typed,
functional language would definitely be advisable, I followed the ad hoc
learning by doing approach and read the sources of the following, publicly
available scheme implementations first. So I would like to thank the
following authors and maintainers for releasing their work:
- [minischeme] really miniature scheme by Atsushi Moriwaki and Cat's Eye Technologies.
- [TinyScheme] by Dimitrios Souflis, Kevin Cozens, Jonathan S. Shapiro and many others.
- [Jscheme] readable Java implementation by Peter Norvig.
- [LispMe] interesting [SECD vm] scheme by Fred Bayer with [sources code].
- [Chibi-Scheme] feature complete [R7RS scheme] by Alex Shinn.

Implementation details
----------------------

### Scheme cells ###
A scheme cell is derived from a std::variant type of all supported types.
Atomic types, like booleans, characters, numbers and symbols are value types and
compound types like strings, vectors, IO-ports or closures are either stored as
shared pointers or by a small handle type with internal pointer to the
implementation class. This assures that the byte size of a scheme cell remains
reasonable small. The largest value types are numbers with 16 bytes for a
complex number as a double floating point pair and 8 bytes for the number
variant internals itself. A scheme cell has therefor a size of 32 bytes in
total (24 bytes + 8 bytes internals). Scheme Cons cells are stored as plain
c-pointers into the global cell store which is basically a std::deque array
of Cons cell pairs.

### Garbage collector ###
todo

[std::variant]: https://en.cppreference.com/w/cpp/utility/variant
[minischeme]:   https://github.com/catseye/minischeme
[TinyScheme]:   https://tinyscheme.sourceforge.net/home.html
[Jscheme]:      https://norvig.com/jscheme.html
[LispMe]:       http://lispme.de/lispme/index_en.html
[Chibi-Scheme]: http://synthcode.com/scheme/chibi
[SECD vm]:      https://en.wikipedia.org/wiki/SECD_machine
[source code]:  https://github.com/arichel/LispMe
[r4rs scheme]:  http://people.csail.mit.edu/jaffer/r4rs_toc.html
[r7rs scheme]:  http://r7rs.org
