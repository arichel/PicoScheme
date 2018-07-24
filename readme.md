
### Scheme cells ###
A scheme cell structure is derived from a std::variant type of all supported types.
Atomic types, like booleans, characters, numbers or symbols are value types and
compound types like strings and vectors are either stored as shared pointers or
by a small handle type with internal pointer to the implementation class. This
assures that the byte size of a cell remains reasonable small.
