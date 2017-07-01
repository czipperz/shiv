# Shiv Spec

This document defines the way the language Shiv should work.

## Word

A Word is a series of characters where the first is an underscore
(`_`) or letter (`a-z` and `A-Z`), and the following characters (if
there are any) are either underscores (`_`), letters (`a-z` and
`A-Z`), or numbers (`0-9`).

Examples:
* `_aBunchOfShortWords`
* `_a_bunch_of_short_words`
* `hash1231valid`

## Namespaced Word

A Namespaced Word is a series of one or more Words separated by two colons (`::`).

Examples:
* `word123`
* `std::i8`
* `mpd::connection::close`

## Type

A Type is either a Builtin Type, a User Defined Type, or a Pointer to
a Qualified Type.

A Builtin Type is one of the types that that the compiler defines
(examples are `std::byte`, `std::void`).

A User Type is a structure or a union defined programmatically.

A Pointer stores the memory address of the Qualified Type it
encapsulates.  A Pointer to a type T is written as `*T`.

## Qualified type

A Qualified Type is a Type that is possibly annotated with the trait
`const`, making it immutable.

Examples:
* A `const` qualified type T is written as `const T`.
* A `const` qualified pointer to a type T is written as `const *T`.
* A pointer to a `const` qualified type T is written as `*const T`.
* A `const` qualified pointer to a `const` qualified type T is written
  as `const *const T`.

## Function definition

    @function_name := fun (@param1 : @type1, ... @paramM-1 : @typeM-1, @paramM : @typeM = @valueM, ..., @paramN : @typeN = @valueN) -> @return_type {
        @statement1;
        @statement2;
        ...
        @statementN;
    }

### Function name

The function name can be any single Namespaced Word.

### Parameters

The parameters are a comma separated list.  Each parameter is either
of the form

    @paramname : @type

or given a default value:

    @paramname : @type = @value

Parameters with a default value must appear after all parameters
without a default value.

Therefore if the parameter M is given a default value, then M+1, M+2,
..., N must have default values.

`@paramname` is a Word.  `@type` is a Qualified Type.

### Return type

Return types are specified after the parameters via a right arrow and
then a Type name.

## Truthy

A expression is Truthy iff it evaluates to an Integer that is non-zero or to a
Pointer that is not null.

## Statement

A statement is either an Expression or an If Statement, While
Statement, For Statement, Return Statement, Goto Statement, Label
Statement, or Variable Declaration Statement.

### If Statement

An If Statement takes the following form:

    if (@condition1) {
        @statement1T1;
        @statement1T2;
        ...
        @statement1TN;
    }

Proceeding the If Statement there may be an Else Clause.

    else {
        @statement1F1;
        @statement1F2;
        ...
        @statement1FN;
    }

or

    else if (@condition2) {
        @statement2T1;
        @statement2T2;
        ...
        @statement2TN;
    }

which may be proceeded by another Else Clause and so on.

If `@condition1` is Truthy, then the Statements `@statement1T` are
ran.  If it is not, the Statements `@statement1F` are ran.

The second form of the Else Clause is just syntactic sugar to allow
the user to drop the extra indentation from the curly braces.

### While Statement

A While Statement takes the following form:

    while (@condition) {
        @statement1;
        @statement2;
        ...
        @statementN;
    }

If `@condition` is Truthy, then the Statements `@statement` are ran
and then repeat.

### Return Statement

    return @statement;

This yields `@statement` as the value of calling this function.

### Goto Statement

    goto @labelname;

This shifts execution to the Label named `@labelname`.

### Label Statement

    label @labelname;

This is where execution shifts to when `goto @labelname;` is executed.
Executing the Label Statement has no effect.

### Variable Declaration Statement

    @varname : @typename;
    @varname : @typename = @value;
    @varname : const = @value;
    @varname := @value;

When specified, `@typename` is the value of the variable `@varname`.

In the last three forms, the result of evaluating `@value` must be the
type of the variable.

## Expression


