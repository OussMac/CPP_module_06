# CPP Module 06

## CPP Reminders

### Casting, and why C++ even bothers with this

in C, you cast like this:

```c
int i = (int)3.9;
```

that's it. one syntax, `(type)expr`, does literally everything. numeric conversion, pointer reinterpretation, stripping `const`, unrelated pointer type punning, all the same syntax. the compiler doesn't discriminate between "safe, well-defined conversion" and "dangerous, implementation-defined bit reinterpretation", it just does whatever it can and doesn't bother

that's the entire problem C++ is solving with its 4 cast operators. same job, but now the _kind_ of conversion you're asking for is baked into the keyword itself, so both the compiler and whoever reads your code afterwards immediately know what category of risk they're looking at.

```cpp
static_cast<int>(3.9);       // "this is a normal, defined conversion"
dynamic_cast<Derived*>(ptr); // "check at runtime if this is safe, tell me if not"
const_cast<int&>(x);         // "I'm only touching const/volatile, nothing else"
reinterpret_cast<int*>(ptr); // "I know this is sketchy, do it anyway"
```

why this matters practically: if you `grep reinterpret_cast` across a codebase, you find every genuinely dangerous conversion in seconds. try doing that with `(type)expr`, you'd have to `grep -E '\([a-zA-Z_]+\s*\*?\)'` and manually eyeball hundreds of false positives (regular parenthesized expressions look identical to a cast). the C-style cast makes intent invisible. the C++ casts make it explicit.

#### the 4 casts, quick reference

|Cast|What it actually does|Checked when|
|---|---|---|
|`static_cast`|conversions the language already knows how to do: numeric types, pointer up-casts in a hierarchy, calling an explicit conversion operator|compile time|
|`dynamic_cast`|safe downcast through a polymorphic hierarchy (needs vtable/RTTI), returns `NULL` (pointers) or throws `std::bad_cast` (references) if the object isn't actually that type|runtime|
|`const_cast`|adds or removes `const`/`volatile`, touches zero bits of the actual value|compile time (no runtime check needed, it's not converting anything)|
|`reinterpret_cast`|reinterprets the raw bit pattern as a different type, no safety net at all|neither, it just trusts you|

#### why ex00 uses static_cast specifically

the subject's Additional Rule says every exercise must pick one casting style and defend the choice. ScalarConverter is converting between `char`, `int`, `float`, `double`, all built-in arithmetic types with conversion rules already defined by the language standard. that is _exactly_ the textbook use case for `static_cast`, there's no polymorphism involved (no `dynamic_cast` needed), nothing const to strip (no `const_cast` needed), and nothing being bit-reinterpreted (no `reinterpret_cast` needed, that one's for ex01's `Serializer`, where a `Data*` genuinely does get reinterpreted as a `uintptr_t`).

using `static_cast` here also means the compiler is still checking my back. if I ever accidentally wrote `static_cast<int>(someUnrelatedPointer)`, it would refuse to compile, a C-style cast would have silently let it through as a `reinterpret_cast` in disguise, and I wouldn't find out until something crashed at runtime.

#### why explicit casts at all, doesn't C do this automatically?

yes, mostly. `int x = someFloat;` compiles fine in both C and C++, implicit narrowing conversions are legal. the subject specifically asks for **explicit** conversion anyway: _"convert it explicitly to the three other data types."_

the point isn't that the compiler needs the cast to know what to do, it's that _I_ need to write it to prove I know what's happening. `static_cast<int>(4.9)` truncates toward zero, on purpose, visibly. an implicit conversion buried in a plain assignment does the exact same truncation silently, and six months later nobody (including me) can tell at a glance whether that was intentional or a bug.

---

### iomanip: std::fixed + std::setprecision

```cpp
std::cout << std::fixed << std::setprecision(1);
std::cout << "float: " << value << "f\n";
```

`std::fixed` is a formatting flag, it tells `ostream` "don't ever switch to scientific notation (`1.23e+05`), always print plain decimal digits." without it, a big enough float will silently print as `1.23457e+08` instead of `123457000.0`, which doesn't match anything the subject's example output shows.

`std::setprecision(1)` controls _how many digits come after the decimal point_ when combined with `std::fixed` (note: without `std::fixed`, `setprecision` means something else entirely, total significant digits, not decimal places, that combination trips people up constantly).

both of these are **sticky manipulators**, once you stream them into `cout`, they stay in effect for every subsequent `<<` until something changes them again. that's why I only set them once inside `printFloat`/`printDouble` rather than re-applying them every single print call.

why precision 1 specifically: every example in the subject (`0.0f`, `-4.2f`, `4.2f`, `42.0f`) shows exactly one digit after the dot. matching that exactly is a deliberate visual choice, not the "true" precision of a float (which has like 6-9 significant decimal digits worth of real precision). setprecision(1) rounds to 1 decimal, so `4.23456f` would print as `4.2f`, that's an accepted simplification, the subject never asks for full precision display.

---

### std::numeric_limits

```cpp
if (value < std::numeric_limits<char>::min()
    || value > std::numeric_limits<char>::max())
```

this replaces the old C macros (`INT_MAX`, `CHAR_MIN`, `FLT_MAX`, ...) from `<limits.h>`/`<float.h>`. those macros are untyped preprocessor text substitution, `numeric_limits` is a proper templated class, so `numeric_limits<char>::min()` and `numeric_limits<int>::min()` are two genuinely different, type-correct calls, the compiler catches you if you use the wrong one for the wrong type, a macro never would.

one thing worth knowing here: whether `char` is signed or unsigned is **implementation-defined** in C++, not something the standard locks down. on basically every platform this project gets compiled/graded on (x86/x86_64 Linux, macOS), `char` is signed, so `numeric_limits<char>::min()` is `-128` and `max()` is `127`. that's the range I'm checking against whenever converting a bigger int/float/double down into a char.

---

### std::isnan / std::isinf (`<cmath>`)

```cpp
if (std::isnan(value) || std::isinf(value))
```

these exist because of one weird IEEE754 property: **NaN is not equal to itself.** `value == value` is `false` if `value` is NaN. so you cannot detect "is this NaN" with a normal comparison, you need the dedicated function.

why this matters here specifically: the subject's pseudo-literals (`nan`, `nanf`, `+inf`, `-inf`, `+inff`, `-inff`) parse through `strtof`/`strtod` into actual IEEE754 NaN/Infinity bit patterns. once that value exists as a `float`/`double`, converting it down to `char` or `int` is meaningless (there's no integer representation of "not a number"), so I check `isnan`/`isinf` first and print `impossible` for those two lines instead of doing undefined-ish comparisons against a value that breaks normal comparison rules.

`isinf` also catches a second, unrelated case: a legitimate huge decimal literal like `99999999999999999999999999999999999999999999999999.0f` that's simply too big to fit in a float. `strtof` doesn't error on that, it just clamps the result to `+infinity`. `isinf` treats that exactly the same as the `+inff` pseudo-literal, which is the correct behavior, once it's infinity, it's infinity, doesn't matter how it got there.

---

### std::strtol / std::strtof / std::strtod (`<cstdlib>`)

the subject explicitly allows this: _"Authorized: Any function to convert from a string to an int, a float, or a double. This will help, but won't do the whole job."_ meaning: yes, use these, but the type **detection**, the **bounds checking**, and the **explicit cross-conversion** between all 4 types is still on me.

```cpp
int value = static_cast<int>(std::strtol(literal.c_str(), NULL, 10));
```

`strtol` returns a `long`, not an `int` (on most 64-bit platforms `long` is bigger than `int`), so I have to `static_cast` it down explicitly. the `10` is the base (decimal, per the subject: _"only the decimal notation will be used"_). the `NULL` is where you'd normally pass a `char**` to get an end-pointer for detecting how much of the string was actually consumed, I don't need that here because `preparser_bounds`/`detection` already validated the string shape _before_ I ever call `strtol`, so by the time I get here I already know it's a clean, fully-numeric string.

`strtof`/`strtod` work the same idea, just return `float`/`double` directly and additionally understand `"nan"`, `"inf"`, `"-inf"` natively as part of the standard, which is genuinely convenient since it means I don't have to hand-roll IEEE754 bit patterns myself, I just let the C library do it and detect the result with `isnan`/`isinf` afterward.

why not `atoi`/`atof`: those are the older, even-more-primitive versions, no way to detect failure at all (return `0` both when the string is `"0"` and when it's garbage), no `endptr`, no base argument. `strtol`/`strtof`/`strtod` are the direct upgrades and are what you're expected to reach for in anything post-C89.

---

### std::isdigit / std::isprint (`<cctype>`)

```cpp
if (!std::isprint(static_cast<unsigned char>(c)))
```

these are character-classification functions from C, carried into C++. the part that trips people up: **the argument must be representable as `unsigned char` or be `EOF`.** `char` is signed on this platform, so a byte like `0xE9` becomes `-23` as a `char`, and handing a negative value (other than `-1`/`EOF`) into `isdigit`/`isprint` is technically undefined behavior per the standard. that's why `printChar` in `convert_utils.cpp` explicitly wraps it: `static_cast<unsigned char>(c)` first, _then_ pass it in.

_(known gap: `sign_check`/`detection` in `ScalarConverter.cpp` call `isdigit`/`isprint` on raw `char` without that cast. in practice this never bites because the subject's inputs are decimal digits and ASCII punctuation, all well within the positive range, but it's worth knowing it's there if a non-ASCII byte ever gets thrown at it.)_

---

### why ScalarConverter can't be instantiated (exercise-specific reasoning, not a repeat of OCF)

this one's not really about OCF mechanics (already covered that in 05), it's about _why this specific class breaks the normal pattern on purpose_. subject: _"As this class doesn't need to store anything at all, it must not be instantiable by users."_

`ScalarConverter` has zero member variables. it exists purely to hold one `static` function. there is no meaningful "object" to construct, ever, `ScalarConverter x;` would just be a value-less placeholder taking up space for no reason. so all four special members (default ctor, copy ctor, copy assignment, **and** the destructor) are `private`, which is one step further than the minimum needed, even locking the destructor means nobody can construct one _anywhere_, not even as a local temporary that immediately dies. it's the C++ equivalent of a namespace full of free functions, dressed up as a class because the subject wants a class.

---

## ex00 : Conversion of scalar types

### the pipeline, top to bottom

```
literal (std::string)
      │
      ▼
┌──────────────┐
│  detection()  │  → figures out WHICH of the 4 types this string represents
└──────┬───────┘
       │
       ▼
┌────────────────────┐
│  preparser_bounds() │  → catches int overflow (2147483648, etc) before strtol touches it
└──────┬─────────────┘
       │
       ▼
┌──────────────────────────────────────┐
│  convertChar/Int/Float/Double()       │  → does the actual strtoX() parse,
│  then explicitly static_casts to      │     then prints all 4 lines
│  the other 3 types                    │
└──────────────────────────────────────┘
```

two completely separate passes on purpose: `detection()` never touches the _value_, only the _shape_ of the string (is it digits-only? does it have exactly one dot? etc). the actual numeric parsing (`strtol`/`strtof`/`strtod`) only happens once, later, in `convert_utils.cpp`, once we already know for certain which type we're dealing with. this split means a malformed string never even reaches the C parsing functions.

### detection() : the entry point

```cpp
static int detection(const std::string& literal)
{
    if (literal.empty())
        return (INVALID);
    if (literal.length() == 1 && std::isprint(literal[0])
        && !std::isdigit(literal[0]))
        return (CHAR);

    if (literal == "nanf" || literal == "+inff" || literal == "-inff")
        return (FLOAT);
    if (literal == "nan" || literal == "+inf" || literal == "-inf")
        return (DOUBLE);

    if (invalid_chars(literal) == INVALID)
        return (INVALID);

    int float_or_double = float_double(literal);
    if (float_or_double == FLOAT)
        return (FLOAT);
    else if (float_or_double == DOUBLE)
       return (DOUBLE);
    else if (integer(literal) == INT)
        return (INT);
    else
        return (INVALID);
}
```

order matters here, each check is a cheap early-exit before the expensive ones:

1. **empty string** → immediately invalid, avoids ever indexing `literal[0]` on nothing.
2. **length-1 printable non-digit** → this is the CHAR shortcut. subject's char examples are bare characters passed on the command line (`./convert a`, not `./convert 'a'`, argv doesn't carry the quotes), so "exactly one character, and it's not a digit" is the whole rule. excluding digits here matters, `./convert 5` must become an `int`, not a `char`, this check is what keeps that correct.
3. **exact pseudo-literal string match** for `nan`/`nanf`/`+inf`/`-inf`/`+inff`/`-inff` → these six are hardcoded literal comparisons because they don't follow the normal digit-dot-digit shape at all, easier to just special-case the exact strings the subject lists than to try to make the general parser understand them.
4. **`invalid_chars()`** → a cheap whitelist pass (digit, `f`, `.`, `+`, `-`, nothing else) that rejects garbage like `"abc"` in one linear scan before wasting time on the more complex float/double state machine.
5. **`float_double()`** then **`integer()`** → the two real parsers, tried in that order because a float/double literal always contains a `.` which an int literal never does, so trying float/double first and falling through to plain integer parsing on failure is the natural order.

### integer() : the simple one

```cpp
static int integer(const std::string& literal)
{
    const char *str = literal.c_str();
    int i = 0;

    if (sign_check(str) == true)
        i++;
    else if (sign_check(str) == false)
        return (INVALID);

    while (str[i])
    {
        if (!std::isdigit(str[i]))
            return (INVALID);
        i++;
    }
    return (INT);
}
```

`sign_check` (shared helper, used by both `integer()` and `float_double()`) confirms the string _starts_ plausibly, a digit, or a sign followed by a digit or dot. `integer()` skips past that first character and then demands every remaining character be a digit, no dot allowed anywhere else. that's it, this is the least complicated function in the file.

### float_double() : where all the actual complexity lives

this is the function doing the heavy lifting, and it's worth walking through in the order it actually executes, not just reading it top to bottom.

**step 1, count dots and f's:**

```cpp
if (fcount > 1 || count != 1 || dot == false)
    return (INVALID);
```

exactly one dot, required (that's what separates float/double from int in the first place), and at most one `f` (the suffix). two dots (`4.2.3`), zero dots, or two `f`s are all immediately rejected here, before anything else runs.

**step 2, `after_f()`, the suffix must be the very last character:**

```cpp
static int after_f(const char *str)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] == 'f')
        {
            if (str[i + 1] != '\0')
                return (1);
        }
        i++;
    }
    return (0);
}
```

if there's an `f` anywhere, and anything comes after it, that's invalid (`4.2f5` shouldn't parse as anything). walks the string looking for `'f'`, and the moment it finds one, checks whether the very next character is the null terminator. if not, the `f` wasn't actually a suffix, it was garbage in the middle, reject.

**step 3, the `digitNextToDot()` fast path, and the bug I found here**

this is a shortcut meant to handle two edge shapes the main loop below doesn't naturally cover well: a **leading dot** with no digit before it (`.5`), and a **trailing dot** with no digit after it (`5.` or `5.f`).

```cpp
static int digitNextToDot(const char *str)
{
    int dot_pos = dot_position(str);
    if (dot_pos == -1)
        return (0);

    int check_place = right_left(str, dot_pos);
    int valid = 0;

    if (check_place == RIGHT)
    {
        valid = valid_right(str, dot_pos);
        if (valid)
            return (valid);
    }
    else if (check_place == LEFT)
    {
        valid = valid_left(str, dot_pos);
        if (valid)
            return (valid);
    }
    return (0);
}
```

`right_left()` looks at where the dot sits and decides which shape we're dealing with: `LEFT` means "everything meaningful is to the right of the dot" (`.5`, leading dot, nothing before it), `RIGHT` means "everything meaningful is to the left of the dot" (`5.`, `5.f`, trailing dot, nothing between the dot and the end/suffix).

`valid_left()` is straightforward, walk forward from just after the dot, confirm it's all digits (optionally ending in `f`), and it can't return early with a wrong answer because it explicitly bails on `'\0'` or `'f'` right at the start if there's _nothing_ after the dot.

`valid_right()` is the one that had a real bug in it, worth documenting here because it's a good example of a subtle off-by-one that doesn't crash, it just quietly returns the wrong answer:

```cpp
// before the fix
static int valid_right(const char *str, int pos)
{
    pos--;
    while (pos >= 0)
    {
        if (str[pos] != '-' && str[pos] != '+' && !std::isdigit(str[pos]))
            return (0);
        pos--;
    }
    if (str[pos] && str[pos + 1] == 'f')   // pos is ALWAYS -1 here
        return (FLOAT);
    return (DOUBLE);
}
```

the loop's whole job is to walk _backward_ from the dot, confirming every character before it is a digit (or a leading sign). the only way that loop exits normally is when `pos` has been decremented all the way past index `0`, to `-1`. that means the `if` check right after the loop was reading `str[-1]`, memory _before_ the string even starts, undefined behavior, every single time this path was reached.

and it wasn't just unsafe, it was logically wrong too: `str[pos + 1]` with `pos == -1` is `str[0]`, the string's very first character. that can never be `'f'` (the string always starts with a digit or a sign), so the condition was permanently false, meaning `valid_right` **always** returned `DOUBLE`, even for something like `"5.f"` which should clearly be `FLOAT`. confirmed with a debug build: `42.0f` detected `FLOAT` correctly (different code path, has a digit between the dot and the `f`), but `42.f` and `5.f` were coming out `DOUBLE`.

the fix: capture the position you actually need to check _before_ the backward-scanning loop starts mutating `pos` for its own unrelated purpose.

```cpp
// after the fix
static int valid_right(const char *str, int pos)
{
    int suffix_pos = pos + 1;   // remember "right after the dot", BEFORE pos moves
    pos--;
    while (pos >= 0)
    {
        if (str[pos] != '-' && str[pos] != '+' && !std::isdigit(str[pos]))
            return (0);
        pos--;
    }
    if (str[suffix_pos] == 'f')
        return (FLOAT);
    return (DOUBLE);
}
```

`suffix_pos` is computed once, from the dot's real position, and never touched again. `str[suffix_pos]` is always a valid index (worst case it's the string's own `'\0'`), no more out-of-bounds read, and the FLOAT/DOUBLE decision is now actually looking at the right character. verified this with AddressSanitizer + UndefinedBehaviorSanitizer across every test in the table below, zero issues, and `42.f`/`5.f`/`-5.f`/`+5.f` now correctly detect as `FLOAT`.

**step 4, the general fallback loop:**

```cpp
while (str[i])
{
    if (!std::isdigit(str[i]) && str[i] != '.')
        return (INVALID);
    if (str[i] == '.')
        break ;
    i++;
}

if (str[i] == '.')
{
    i++;
    if (str[i] == '\0' || str[i] == 'f')
        return(INVALID);
}
while (str[i])
{
    if (!std::isdigit(str[i]) && str[i] != 'f')
        return (INVALID);
    if (str[i] == 'f')
        break ;
    i++;
}
if (str[i] == 'f' && !str[i + 1])
    return (FLOAT);
else if (!str[i])
    return (DOUBLE);
return (INVALID);
```

this is the "normal shape" handler: digit(s), dot, digit(s), optional `f`. first loop walks up to the dot confirming digits-only. second block confirms there's _at least one_ digit after the dot (rejects `5.f`/`5.` here too, but those never reach this far because `digitNextToDot` already caught them earlier). third loop walks the fractional digits to the end, and the final check decides FLOAT (ends in `f` with nothing after) vs DOUBLE (ends in `'\0'` directly).

### preparser_bounds() / checkIntBounds() : catching int overflow before strtol

```cpp
static int checkIntBounds(const std::string& literal)
{
    std::string number = literal;
    bool negative = false;

    if (number[0] == '+' || number[0] == '-')
    {
        negative = (number[0] == '-');
        number.erase(0, 1);
    }

    while (number.length() > 1 && number[0] == '0')
        number.erase(0, 1);

    if (!negative)
    {
        const std::string max = "2147483647";
        if (number.length() > max.length())
            return INVALID;
        if (number.length() == max.length() && number > max)
            return INVALID;
    }
    else
    {
        const std::string min = "2147483648";  // magnitude of INT_MIN
        if (number.length() > min.length())
            return INVALID;
        if (number.length() == min.length() && number > min)
            return INVALID;
    }
    return INT;
}
```

why string comparison instead of just letting `strtol` handle it: `strtol` _does_ detect overflow, but only by setting `errno` to `ERANGE` and clamping the return value to `LONG_MAX`/`LONG_MIN`, you'd have to `errno = 0` beforehand, call it, then check `errno` afterward, and you'd already have called it once by that point (which the whole point of `preparser_bounds` is to avoid, reject bad input _before_ touching the actual conversion function).

instead, this compares the literal's digit-string directly against the string `"2147483647"` (`INT_MAX`) or `"2147483648"` (magnitude of `INT_MIN`, since `-2147483648` is valid but `+2147483648` isn't): strip the sign, strip leading zeros (so `"007"` doesn't get wrongly flagged as longer than it needs to be), then compare lengths first (a 15-digit number is obviously bigger than a 10-digit `INT_MAX` without needing to look at the actual digits), and only fall into an actual string comparison when the lengths tie, where normal lexicographic string comparison happens to give the correct numeric ordering _because_ both strings are the same length and contain only digits.

**float/double overflow is deliberately NOT bounds-checked** the same way. that's a conscious choice, not an oversight, if a decimal literal is too big for a `float`/`double`, `strtof`/`strtod` just clamps it to `±infinity`, which then gets caught by the exact same `isinf()` check used for the `+inff`/`-inf` pseudo-literals. adding a separate manual bounds check here would just be re-detecting the same "it's infinity now" condition through a second, redundant code path.

### convert_utils.cpp : the actual conversion + display layer

once `detection()` + `preparser_bounds()` have decided the type, one of these four runs:

```cpp
void convertChar(const std::string& literal)
{
    char value = literal[0];
    printChar(value);
    printInt(static_cast<int>(value));
    printFloat(static_cast<float>(value));
    printDouble(static_cast<double>(value));
}
```

the char case is the simplest, `literal[0]` already _is_ the value, no parsing function needed at all, just three explicit `static_cast`s up to the wider types (a `char` can always represent exactly in `int`/`float`/`double`, no bounds check needed going _up_).

```cpp
void convertInt(const std::string& literal)
{
    int value = static_cast<int>(std::strtol(literal.c_str(), NULL, 10));

    if (value < std::numeric_limits<char>::min()
        || value > std::numeric_limits<char>::max())
        std::cout << "char: impossible\n";
    else
        printChar(static_cast<char>(value));

    printInt(value);
    printFloat(static_cast<float>(value));
    printDouble(static_cast<double>(value));
}
```

going _down_ from int to char needs the range check, `200` doesn't fit in a signed char. going _up_ to float/double never needs a check, every 32-bit int value is representable (with possible precision loss past ~2^24 for float specifically, but never "impossible", just imprecise, which the subject doesn't ask us to flag).

```cpp
void convertFloat(const std::string& literal)
{
    float value = std::strtof(literal.c_str(), NULL);

    if (std::isnan(value) || std::isinf(value))
    {
        std::cout << "char: impossible\n";
        std::cout << "int: impossible\n";
    }
    else
    {
        // range-checked static_casts down to char and int
    }
    printFloat(value);
    printDouble(static_cast<double>(value));
}
```

`convertFloat`/`convertDouble` follow the same shape: parse with `strtof`/`strtod`, guard the `nan`/`inf` case first (since comparisons against those are meaningless), then range-check against `numeric_limits<char>` and `numeric_limits<int>` before casting down, exactly like `convertInt` does for char. going from float to double (or double to float) never fails, just an explicit `static_cast<double>`/`static_cast<float>` for precision-widening or narrowing.

---

### the test sheet

ran the fixed implementation through 63 inputs, official subject examples, the specific inputs that exposed the `valid_right` bug, pseudo-literals, int/char boundary values, and a batch of deliberately malformed garbage. all under the exact Makefile flags (`-std=c++98 -Wall -Wextra -Werror`, zero warnings) and swept again separately under AddressSanitizer + UndefinedBehaviorSanitizer (zero issues).

<embedded_test_table>

| Input                                                   | Category                                                  | Expected Type | Detected Type | Result | Actual Output                                                                                                       |
| ------------------------------------------------------- | --------------------------------------------------------- | ------------- | ------------- | ------ | ------------------------------------------------------------------------------------------------------------------- |
| `0`                                                     | PDF example                                               | INT           | INT           | ✅ PASS | char: Non displayable \| int: 0 \| float: 0.0f \| double: 0.0                                                       |
| `nan`                                                   | PDF example                                               | DOUBLE        | DOUBLE        | ✅ PASS | char: impossible \| int: impossible \| float: nanf \| double: nan                                                   |
| `42.0f`                                                 | PDF example                                               | FLOAT         | FLOAT         | ✅ PASS | char: '*' \| int: 42 \| float: 42.0f \| double: 42.0                                                                |
| `42.f`                                                  | Bug-fix verification                                      | FLOAT         | FLOAT         | ✅ PASS | char: '*' \| int: 42 \| float: 42.0f \| double: 42.0                                                                |
| `5.f`                                                   | Bug-fix verification                                      | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: 5 \| float: 5.0f \| double: 5.0                                                       |
| `-5.f`                                                  | Bug-fix verification                                      | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: -5 \| float: -5.0f \| double: -5.0                                                    |
| `+5.f`                                                  | Bug-fix verification                                      | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: 5 \| float: 5.0f \| double: 5.0                                                       |
| `16777217.f`                                            | Bug-fix verification                                      | FLOAT         | FLOAT         | ✅ PASS | char: impossible \| int: 16777216 \| float: 16777216.0f \| double: 16777216.0                                       |
| `5.`                                                    | Bug-fix verification                                      | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 5 \| float: 5.0f \| double: 5.0                                                       |
| `-5.`                                                   | Bug-fix verification                                      | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: -5 \| float: -5.0f \| double: -5.0                                                    |
| `+5.`                                                   | Bug-fix verification                                      | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 5 \| float: 5.0f \| double: 5.0                                                       |
| `nanf`                                                  | Pseudo-literal                                            | FLOAT         | FLOAT         | ✅ PASS | char: impossible \| int: impossible \| float: nanf \| double: nan                                                   |
| `+inf`                                                  | Pseudo-literal                                            | DOUBLE        | DOUBLE        | ✅ PASS | char: impossible \| int: impossible \| float: inff \| double: inf                                                   |
| `-inf`                                                  | Pseudo-literal                                            | DOUBLE        | DOUBLE        | ✅ PASS | char: impossible \| int: impossible \| float: -inff \| double: -inf                                                 |
| `+inff`                                                 | Pseudo-literal                                            | FLOAT         | FLOAT         | ✅ PASS | char: impossible \| int: impossible \| float: inff \| double: inf                                                   |
| `-inff`                                                 | Pseudo-literal                                            | FLOAT         | FLOAT         | ✅ PASS | char: impossible \| int: impossible \| float: -inff \| double: -inf                                                 |
| `-42`                                                   | Int                                                       | INT           | INT           | ✅ PASS | char: Non displayable \| int: -42 \| float: -42.0f \| double: -42.0                                                 |
| `+42`                                                   | Int                                                       | INT           | INT           | ✅ PASS | char: '*' \| int: 42 \| float: 42.0f \| double: 42.0                                                                |
| `007`                                                   | Int - leading zeros                                       | INT           | INT           | ✅ PASS | char: Non displayable \| int: 7 \| float: 7.0f \| double: 7.0                                                       |
| `2147483647`                                            | Int - INT_MAX                                             | INT           | INT           | ✅ PASS | char: impossible \| int: 2147483647 \| float: 2147483648.0f \| double: 2147483647.0                                 |
| `-2147483648`                                           | Int - INT_MIN                                             | INT           | INT           | ✅ PASS | char: impossible \| int: -2147483648 \| float: -2147483648.0f \| double: -2147483648.0                              |
| `2147483648`                                            | Int - overflow                                            | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `-2147483649`                                           | Int - overflow                                            | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `99999999999999999999`                                  | Int - gross overflow                                      | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `4.2`                                                   | Double                                                    | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 4 \| float: 4.2f \| double: 4.2                                                       |
| `-4.2`                                                  | Double                                                    | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: -4 \| float: -4.2f \| double: -4.2                                                    |
| `0.0`                                                   | Double                                                    | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 0 \| float: 0.0f \| double: 0.0                                                       |
| `-0.0`                                                  | Double - negative zero                                    | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 0 \| float: -0.0f \| double: -0.0                                                     |
| `.5`                                                    | Double - leading dot                                      | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 0 \| float: 0.5f \| double: 0.5                                                       |
| `-.5`                                                   | Double - leading dot                                      | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 0 \| float: -0.5f \| double: -0.5                                                     |
| `+.5`                                                   | Double - leading dot                                      | DOUBLE        | DOUBLE        | ✅ PASS | char: Non displayable \| int: 0 \| float: 0.5f \| double: 0.5                                                       |
| `4.2f`                                                  | Float                                                     | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: 4 \| float: 4.2f \| double: 4.2                                                       |
| `-4.2f`                                                 | Float                                                     | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: -4 \| float: -4.2f \| double: -4.2                                                    |
| `.5f`                                                   | Float - leading dot                                       | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: 0 \| float: 0.5f \| double: 0.5                                                       |
| `-.5f`                                                  | Float - leading dot                                       | FLOAT         | FLOAT         | ✅ PASS | char: Non displayable \| int: 0 \| float: -0.5f \| double: -0.5                                                     |
| `a`                                                     | Char                                                      | CHAR          | CHAR          | ✅ PASS | char: 'a' \| int: 97 \| float: 97.0f \| double: 97.0                                                                |
| `Z`                                                     | Char                                                      | CHAR          | CHAR          | ✅ PASS | char: 'Z' \| int: 90 \| float: 90.0f \| double: 90.0                                                                |
|                                                         | Char - space                                              | CHAR          | CHAR          | ✅ PASS | char: ' ' \| int: 32 \| float: 32.0f \| double: 32.0                                                                |
| `.`                                                     | Char - lone dot                                           | CHAR          | CHAR          | ✅ PASS | char: '.' \| int: 46 \| float: 46.0f \| double: 46.0                                                                |
| `+`                                                     | Char - lone plus                                          | CHAR          | CHAR          | ✅ PASS | char: '+' \| int: 43 \| float: 43.0f \| double: 43.0                                                                |
| `-`                                                     | Char - lone minus                                         | CHAR          | CHAR          | ✅ PASS | char: '-' \| int: 45 \| float: 45.0f \| double: 45.0                                                                |
| `5`                                                     | Single digit → INT not CHAR                               | INT           | INT           | ✅ PASS | char: Non displayable \| int: 5 \| float: 5.0f \| double: 5.0                                                       |
| `(empty string)`                                        | Invalid - empty string                                    | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `abc`                                                   | Invalid - letters                                         | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `12a`                                                   | Invalid - mixed garbage                                   | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `4.2.3`                                                 | Invalid - two dots                                        | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `4..2`                                                  | Invalid - adjacent dots                                   | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `ff`                                                    | Invalid - letters only                                    | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `4.2ff`                                                 | Invalid - double f                                        | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `4.f2`                                                  | Invalid - char after f                                    | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `1e10`                                                  | Invalid - scientific notation (unsupported by subject)    | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `nan42`                                                 | Invalid - malformed pseudo-literal                        | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `-inf42`                                                | Invalid - malformed pseudo-literal                        | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `--5`                                                   | Invalid - double sign                                     | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `5-`                                                    | Invalid - trailing sign                                   | INVALID       | INVALID       | ✅ PASS | invalid                                                                                                             |
| `200`                                                   | Int → char impossible                                     | INT           | INT           | ✅ PASS | char: impossible \| int: 200 \| float: 200.0f \| double: 200.0                                                      |
| `-200`                                                  | Int → char impossible                                     | INT           | INT           | ✅ PASS | char: impossible \| int: -200 \| float: -200.0f \| double: -200.0                                                   |
| `127`                                                   | Int → char boundary (CHAR_MAX)                            | INT           | INT           | ✅ PASS | char: Non displayable \| int: 127 \| float: 127.0f \| double: 127.0                                                 |
| `-128`                                                  | Int → char boundary (CHAR_MIN)                            | INT           | INT           | ✅ PASS | char: Non displayable \| int: -128 \| float: -128.0f \| double: -128.0                                              |
| `128`                                                   | Int → char just above boundary                            | INT           | INT           | ✅ PASS | char: impossible \| int: 128 \| float: 128.0f \| double: 128.0                                                      |
| `-129`                                                  | Int → char just below boundary                            | INT           | INT           | ✅ PASS | char: impossible \| int: -129 \| float: -129.0f \| double: -129.0                                                   |
| `99999999999999999999999999999999999999999999999999.0`  | Double overflow → inf (design choice: not bounds-checked) | DOUBLE        | DOUBLE        | ✅ PASS | char: impossible \| int: impossible \| float: inff \| double: 100000000000000007629769841091887003294964970946560.0 |
| `99999999999999999999999999999999999999999999999999.0f` | Float overflow → inf                                      | FLOAT         | FLOAT         | ✅ PASS | char: impossible \| int: impossible \| float: inff \| double: inf                                                   |


**63/63.** compiled clean (`c++ -std=c++98 -Wall -Wextra -Werror`, 0 warnings), 0 ASan/UBSan issues across the whole sweep, and the 3 official PDF examples match byte-for-byte.

### Questions typically asked for ex00

| Question                                                                                       | Answer                                                                                                                                                                                                                                                                                                                                                                       |
| ---------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Why must ScalarConverter be non-instantiable?                                                  | It has zero state, subject explicitly says it "doesn't need to store anything at all", so there's nothing a real object of this class would ever hold.                                                                                                                                                                                                                       |
| Why static_cast and not a C-style cast?                                                        | static_cast documents that this is a defined, compiler-checked numeric conversion. A C-style cast would compile the exact same code but silently permit a reinterpret_cast-style conversion too, with zero warning if I ever wrote one by mistake.                                                                                                                           |
| Why detect the type before parsing instead of just trying strtod on everything?                | strtod would happily "succeed" on a prefix of garbage input (`"42abc"` parses as `42.0` and just leaves `abc` unconsumed). Detecting the shape first means malformed input is rejected outright instead of silently truncated.                                                                                                                                               |
| Why isnan/isinf instead of comparing value != value or value > SOME_MAX?                       | NaN breaks normal comparison operators (NaN != NaN is true), isnan/isinf are the only correct way to detect it.                                                                                                                                                                                                                                                              |
| Why does int bounds-checking use string comparison instead of just catching strtol's overflow? | Avoids the errno dance, and rejects the bad literal before ever calling strtol at all, keeping the "detect first, parse second" separation clean.                                                                                                                                                                                                                            |
| Why isn't float/double overflow bounds-checked the same way int is?                            | Deliberate: strtof/strtod already clamp overflow to infinity on their own, and that gets caught by the same isinf() check used for the inf pseudo-literals, a separate manual check would just duplicate that.                                                                                                                                                               |
