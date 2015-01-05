# µjson
## About

µjson is a a small, C++11, UTF-8, JSON library.

Its highlights are:

* Small library with very simple API
* Outputs nicely formatted JSON
* Fast UTF-8 conformant parser
* Liberal license

## Dependencies

The library uses the double-conversion library from the
[V8 JavaScript engine](https://code.google.com/p/double-conversion)
for portable conversion between ASCII and floating point numbers. An
amalgamation of v1.1.5 of this library is included in the source
distribution.

Unit tests are written using
[Catch](https://github.com/philsquared/Catch). Catch is also included
in the source distribution.

The scanner is generated using [re2c](http://re2c.org). The source
distribution includes the generated file, so this tool is only needed
if you intend to modify the scanner.

## Licenses

µjson is licensed under the MIT license. See the `LICENSE.md` file in the
source distribution.

The dependencies all have liberal software licenses. See
`LICENSE-3RD-PARTY.md` for the details.

## Installation

The library, examples, and unit tests can be built using
[CMake](http://www.cmake.org). The CMake scripts will automatically
download `re2c`.

When using the library in another project, rather than using CMake, it
may be easier to simply include the four source files,
````
ujson.hpp
ujson.cpp
double-conversion.h
double-conversion.cc,
````
directly in the project.

## Tutorial

Consider representing books defined using this simple struct as JSON:
````cpp
struct book_t {
    std::string title;
    std::vector<std::string> authors;
    int year;
};
````
The first step is to write a small function for converting a book into a
`ujson::value`:
````cpp
ujson::value to_json(book_t const &b) {
    return ujson::object{ { "title", b.title },
                          { "authors", b.authors },
                          { "year", b.year } };
}
````
Using the above function an array of books can be converted to JSON as
follows:
````cpp
book_t book1{ "Elements of Programming",
              2009,
              { "Alexander A. Stepanov", "Paul McJones" } };
book_t book2{ "The C++ Programming Language, 4th Edition",
              2013,
              { "Bjarne Stroustrup" } };
std::vector<book_t> book_list{ book1, book2 };

ujson::value value{ book_list };
std::string json = to_string(value);
std::cout << json << std::endl;
````
The last line will print:
````json
[
    {
        "authors" : [
            "Alexander A. Stepanov",
            "Paul McJones"
        ],
        "title" : "Elements of Programming",
        "year" : 2009
    },
    {
        "authors" : [
            "Bjarne Stroustrup"
        ],
        "title" : "The C++ Programming Language, 4th Edition",
        "year" : 2013
    }
]
````
Reconstructing the list of books is done by first parsing the JSON
string into a `ujson::value`:
````cpp
ujson::value new_value = ujson::parse(json);
assert(new_value == value);
````
Each element in this array is then converted to a `book_t`:
````cpp
std::vector<ujson::value> array = array_cast(std::move(new_value));
std::vector<book_t> new_book_list;
new_book_list.reserve(array.size());
for (auto it = array.begin(); it != array.end(); ++it)
    new_book_list.push_back(make_book(std::move(*it)));
assert(new_book_list == book_list);
````
The helper function `make_book` is implemented as follows:
````cpp
book_t make_book(ujson::value v) {

    if (!v.is_object())
        throw std::invalid_argument("object expected for make_book");

    book_t book;
    std::vector<std::pair<std::string, ujson::value>> object =
        object_cast(std::move(v));

    auto it = find(object, "title");
    if (it == object.end() || !it->second.is_string())
        throw std::invalid_argument("'title' with type string not found");
    book.title = string_cast(std::move(it->second));

    it = find(object, "authors");
    if (it == object.end() || !it->second.is_array())
        throw std::invalid_argument("'authors' with type array not found");
    std::vector<ujson::value> array = array_cast(std::move(it->second));
    book.authors.reserve(array.size());
    for (auto it = array.begin(); it != array.end(); ++it) {
        if (!it->is_string())
            throw std::invalid_argument("'authors' must be array of strings");
        book.authors.push_back(string_cast(std::move(*it)));
    }

    it = find(object, "year");
    if (it == object.end() || !it->second.is_number())
        throw std::invalid_argument("'year' with type number not found");
    book.year = int32_cast(it->second);

    return book;
}
````

## Reference

A JSON value must be null, a boolean, a number, a string, an array, or
an object ([see RFC7159](http://tools.ietf.org/html/rfc7159)). In
µjson the class `ujson::value` is used to represent all of these six
types.

The actual type of a value can queried using `ujson::value::type` or
using one of the convenience methods, such as `ujson::value::is_null`.
Values always contain one of the six possible types (`ujson::value`
does not have a special uninitialized state).

The class `ujson::value` is a proper immutable value. Therefore, once
a value has been created, it cannot be changed, though of course it
can be assigned a new value. Values can be compared for equality and
inequality.

Casts are used to extract the embedded type again. For instance
`bool_cast` is used to extract the `bool` from values with boolean
types. If the value is cast to a wrong type a bad_cast exception is
thrown.

### Null
Default constructed values are null:
````cpp
ujson::value null_value; // null
````
A constant `null` value is defined in the `ujson` namespace.
````cpp
assert(ujson::null == null_value);
````
Values support stream i/o:
````cpp
std::cout << null_value << std::endl; // prints 'null'
````

### Booleans
Values can be initialized with and assigned `bool`s:
````cpp
ujson::value boolean(true);
assert(bool_cast(boolean) == true);
std::cout << boolean << std::endl; // prints 'true'
boolean = false;
assert(bool_cast(boolean) == false);
std::cout << boolean << std::endl; // prints 'false'
````

### Numbers
Inside `ujson::value`s numbers are represented as 64-bit doubles:
````cpp
ujson::value number = M_PI;
std::cout << number << std::endl; // prints '3.141592653589793'
````
The double value can be extracted using a `double_cast`:
````cpp
double d = double_cast(number); // d == M_PI
````
The double-conversion library is used instead of the platform specific C
runtime library to ensure lossless and portable roundtripping of doubles from
ASCII to binary.

Beware that only finite numbers are valid in JSON. Infinities and NaNs
are not allowed:
````cpp
number = std::numeric_limits<double>::infinity(); // throws bad_number
````

Numbers can also represent signed 32-bit integers:
````cpp
number = 1024;
std::cout << number << std::endl; // prints '1024'
````
The integer value can be extracted using an `int32_cast`:
````cpp
std::int32_t i = int32_cast(number); // i == 1024
````
Unsigned 32-bit integers are also supported.

### Strings
Strings are stored internally as UTF-8:
````cpp
ujson::value value = "\xC2\xA9 ujson 2014"; // copyright symbol
````
If the string is not zero-terminated or contains embedded zeros, the
length must be passed too:
````cpp
char title[]= { 0xC2, 0xB5, 'j', 's', 'o', 'n' }; // micro sign + json
value = ujson::value(title, 6);
````
Strings passed to to µjson must be valid UTF-8:
````cpp
value = "\xF5"; // invalid utf-8; throws bad_string
````
If the string is known to be valid UTF-8, the validation step can be
skipped by passing no in the last argument of the constructor:
````cpp
value = ujson::value("valid", 5, ujson::validate_utf8::no);
````
Strings can also be constructed from `std::string`s:
````cpp
std::string string("ujson");
value = string; // copy into value
````
Alternatively, if the original string is no longer needed, the
`std::string` can be moved into the value and the copy avoided:
````cpp
value = std::move(string); // move into value
````

Strings can be accessed using the two `string_cast` methods. The first
accepts l-values and returns a `ujson::string_view` object:
````cpp
auto view = string_cast(value);
std::cout << view.c_str() << std::endl; // prints 'ujson'
````
The returned string view object provides read-only access to the
contained string.

The second string cast method accepts r-values and can be used to move
a string out of a value:
````cpp
string = string_cast(std::move(value)); // move string out of value
assert(value.is_null());
````
Moved from values are always null.

See the "Implementation Details" section for more information on how
µjson handles `std::string`s implemented using reference counting
versus short string optimization.

### Arrays

Arrays are represented using `ujson::array`, which is simply a typedef for
`std::vector<ujson::value>`:
````cpp
auto array = ujson::array{ true, M_PI, "a string" };
ujson::value value(array);
````
Copying the array can be avoided by moving it into the value:
````cpp
value = std::move(array);
````
Read-only access to the contained array is possible using `array_cast`:
````cpp
ujson::array const &ref = array_cast(value);
````
The original array can be recovered by moving the array out of the value:
````cpp
array = array_cast(std::move(value));
````
As shown in the tutorial it is also possible to use a `std::vector<T>`
of types `T` implicitly convertable to `ujson::value` or a vector of
types that supply a `to_json` function.

`ujson::value`s are designed to be cheap to copy. Internally, strings,
arrays, and objects, are stored using `std::shared_ptr<>`s, so copying
only requires incrementing a reference count. However, this sharing
has implications for when it is possible to move:
````cpp
ujson::value value1 = std::move(array);
ujson::value value2 = value1; // value2 shares immutable array with value1
auto tmp1 = array_cast(std::move(value1)); // note: copy!
auto tmp2 = array_cast(std::move(value2)); // move
````
In short, moves are only possible if the value has exclusive ownership
of the resource. Recall that moved from values are null, so therefore
the last move will succeed.

### Objects

Objects are represented using `ujson::object`, which is simply a
typedef for `std::vector<std::pair<std::string, ujson::value>>`:
````cpp
auto object =
    ujson::object{ { "a null", ujson::null },
                   { "a bool", true },
                   { "a number", M_LN2 },
                   { "a string", "Hello, world!" },
                   { "an array", ujson::array{ 1, 2, 3 } } };
ujson::value value(object);
````
As usual, copies can be avoided by moving:
````cpp
value = std::move(object);
````
Read-only access to the contained object is possible using `object_cast`:
````cpp
ujson::object const &ref = object_cast(value);
````
The original object can be recovered by moving the object out of the value:
````cpp
object = object_cast(std::move(value));
````
For performance reasons objects are implemented using a simple
`std::vector` rather than a `std::map`. However, objects can still be
constructed using a `std::map<std::string,T>` of types `T` implicitly
convertable to `ujson::value` or a map of types that supply a
`to_json` function.

When an `ujson::object` is copied or moved into an `ujson::value` the vector
is sorted, so that lookups can be performed using a binary search:
````cpp
auto it = find(object, "a number");
assert(it->second == M_LN2);
````
In addition to `ujson::find`, there is also a `ujson::at` function
which behaves like `std::map::at`.

Beware that names in objects must also be valid UTF-8:
````cpp
object.push_back({ "invalid utf-8: \xFF", ujson::null });
value = object; // throws bad_string
````

### Reading JSON

Call `ujson::parse` to parse a buffer with UTF-8 encoded JSON:
````cpp
auto value = ujson::parse("[ 1.0, 2.0, 3.0 ]");
````
If the buffer is not zero-terminated, which is the case with e.g. memory mapped
files, the length must also be supplied:
````cpp
const char *mapped_buffer = ..;
std::size_t mapped_length = ..;
auto value = ujson::parse(mapped_buffer, mapped_length);
````
Exceptions are thrown on syntax errors:
````cpp
try {
    auto value = ujson::parse("[ 1.0, 2.0, 3.0 "); // invalid syntax
    ...
} catch (std::exception const &e) {
    std::cout << e.what() << std::endl; // prints 'Invalid syntax on line 1.'
}
````
Apart from syntax errors, the parser will also throw if a number is
too large to fit in a double, if a string contains invalid UTF-8, and if
the buffer contains trailing junk.

### Writing JSON

`ujson::value`s can be converted to JSON using `ujson::to_string`:
````cpp
auto array = ujson::array{ true, 1.0, "Sk\xC3\xA5l! \xF0\x9F\x8D\xBB" };
auto object =
    ujson::object{ { "a null", ujson::null },
                   { "a bool", false },
                   { "a number", 1.61803398875 },
                   { "a string", "R\xC3\xB8""dgr\xC3\xB8""d med fl\xC3\xB8""de." },
                   { "an array", array } };
std::cout << to_string(object) << std::endl;
````
This produces:
````
{
    "a bool" : false,
    "a null" : null,
    "a number" : 1.61803398875,
    "a string" : "Rødgrød med fløde.",
    "an array" : [
        true,
        1,
        "Skål! 🍻"
    ]
}
````
By default µjson indents by four spaces. It's possible change this and
also control whether UTF-8 is allowed in the output:

````cpp
ujson::to_string_options compact_ascii;
compact_ascii.indent_amount = 0;
compact_ascii.encoding = ujson::character_encoding::ascii;
std::cout << to_string(object, compact_ascii) << std::endl;
````
With ASCII output, all non-ASCII characters are escaped and with zero
indentation all insignificant white space is elided:
````
{"a bool":true,"a null":null,"a number":1.61803398875,"a string":"R\u00F8dgr\u00F8d med fl\u00F8de.","an array":[true,1,"Sk\u00E5l! \uD83C\uDF7B"]}
````

## Implementation Details

`ujson::value` is implemented using small object optimiziation. This
avoids the need for expensive heap allocations for simple types, since
the value instead is stored directly inside the object.

| type     | heap allocation  |
|:--------:|:----------------:|
| null     | no               |
| boolean  | no               |
| number   | no               |
| string   | depends          |
| array    | yes              |
| object   | yes              |

Arrays and objects do require heap allocations, since they are stored
internally using a `std::shared_ptr` (usually just a single allocation
is required, since most STL implementations allocate the object and
control block together). While this does make construction more
expensive, it has the advantage that copying values containing arrays
or objects is cheap, since it only amounts to incrementing a reference
count.

Also, since the reference count used by `std::shared_ptr` is
thread-safe and the pointed to value immutable, passing a
`ujson::value` *by value* to another thread is free from race
conditions:

````cpp
auto value = ujson::parse(...);
auto future = std::async(std::launch::async, [value] {
    /* do significant work */ });
````

Strings in the Standard Template Library are implemented using either
short string optimization (SSO) or reference counting. Clang's libc++
and Visual Studio uses the former approach while GCC's libstdc++ uses
the latter. Briefly, in an implementation using reference counting, a
`std::string` stores a pointer to the string data and a reference
count. Copy on write (COW) is used to ensure that a string gets it own
unique copy of the string data if modified. In an implementation using
SSO, the string object stores a pointer and a small buffer. Short
strings are stored in the buffer, thus avoiding the heap allocation,
whereas longer strings are stored on the heap. The size of the buffer
for short strings is implementation defined.  See the 'sso buffer
size' column in the following table.

| platform                |  arch  | ujson::value | std::string | sso buffer size |
|:-----------------------:|:------:|:------------:|:-----------:|:---------------:|
| clang 3.4 (Xcode 5.1.1) | 32-bit | 16 bytes     | 12 bytes    | 10 bytes        |
| clang 3.4 (Xcode 5.1.1) | 64-bit | 32 bytes     | 24 bytes    | 22 bytes        |
| gcc 4.8.3 (via brew)    | 32-bit | 12 bytes     |  4 bytes    | N/A             |
| gcc 4.8.3 (via brew)    | 64-bit | 24 bytes     |  8 bytes    | N/A             |
| vs2013 update 3         | 32 bit | 24 bytes     | 28 bytes    | 15 bytes        |
| vs2013 update 3         | 64 bit | 32 bytes     | 40 bytes    | 15 bytes        |
| vs2013 ctp1             | 32-bit | 24 bytes     | 28 bytes    | 15 bytes        |
| vs2013 ctp1             | 64-bit | 32 bytes     | 32 bytes    | 15 bytes        |


With a COW `std::string` µjson simply stores the string object inside
inside the `ujson::value` without doing any allocations. Copying is
still inexpensive since copying COW strings is cheap.

With a SSO `std::string` short strings are stored directly in the
`ujson::value` object and therefore do not require any heap
allocations. Long strings are stored using a `std::shared_ptr`, so
they require a single allocation. Like arrays and objects, copying
long strings is therefore cheap.

In summary, copy constructing and copy assigning `ujson::value`s is
always an inexpensive operation, requiring at most bumping a
reference count or copying a small buffer, but never any heap
allocations.
