# Conventions

None of these things are absolute; there's always room for special cases with
special considerations. However, these coding conventions should describe
how this code base generally looks and works.

## Superficial

* Files are indented using tabs.
* Opening curly braces go at the end of the line.
* Never align, just create new indentation levels; for example, a multi-line
  function call should look like this:

 ``` C++
SomeNamespace::someLongFunctionName(
	SomePossiblyVeryLongExpression(),
	10, 20);
```

* There's a "soft" line length limit of 80 columns; lines can be longer, but
  not significantly longer.
* Multi-line function parameter lists are indented twice, to visually separate
  them from the body:

``` C++
void someLongFunctionName(
		int firstArgument,
		int secondArgument) {
	whatever;
}
```

## Names

* Classes and structs are PascalCase.
* Methods and free functions are camelCase.
* Local variables are camelCase.
* Class member variables are camelCase\_ (with the trailing underscore).
* Struct member variables are camelCase (without the trailing underscore).
* Constants are UPPER\_SNAKE\_CASE (no trailing underscore) everywhere.

## Structure

* Classes are structured with public first, then protected, then private.
* Each section (public, protected, private) of a class starts with the member functions,
  then the static member functions, then the member variables.
* Structs don't contain private or protected fields.
* Structs starts with the member variables, then member functions, then static member functions.
* Headers are named `*.h`, source files are named `*.cc`.

## Behaviour

* In general, use `unique_ptr` and `make_unique` for owning pointers.
* References and raw pointers should always be non-owning.
* Only use `shared_ptr` when you can't get away from shared ownership.
* In the case of cycles, try to make ownership hierarchical; e.g the parent
  has a `unique_ptr` to the child, while the child only has a reference
  or raw pointer to the parent.
* When the parent and child are in different headers, the child's header should
  forward declare the parent, while the parent's header can include the child's
  header. Cyclic includes are evil.
* In source files which act as the implementation of a header, the very first
  substantial line should include the corresponding header.
* All headers should have a corresponding source file, even if that source file
  is empty except for the include. This is to avoid include ordering issues.

## Rationale

While a lot of this is just because we need to standardize on _something_,
some of the points have pretty good reasons.

* I view structs as "mainly data"; the reason you have a struct is to conveniently
  put different variables together. Classes, on the other hand, are "mainly behaviour",
  so their member variables (to the degree that they have public members at all)
  should be de-emphasized.
* Classes should start with their public members because that's the only thing
  other components care about. It's odd to hide away the public interface behind
  a wall of implementation details.
