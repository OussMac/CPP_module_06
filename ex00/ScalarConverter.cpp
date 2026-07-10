#include "ScalarConverter.hpp"

ScalarConverter::ScalarConverter() {}

ScalarConverter::ScalarConverter(const ScalarConverter &other) {
  (void)other;
}

ScalarConverter &ScalarConverter::operator=(const ScalarConverter &other) {
    (void)other;
  return *this;
}

ScalarConverter::~ScalarConverter() {}

static bool sign_check(const char *str)
{
    if (std::isdigit(str[0]))
        return (true);
    if ((str[0] == '-' || str[0] == '+') && std::isdigit(str[1]))
        return (true);
    return (false);
}

static int float_double(const std::string& literal)
{
    bool dot = false;
    int count = 0;
    const char *str = literal.c_str();

    for (int i = 0; str[i]; i++)
    {
        if (str[i] == '.')
        {
            count++;
            dot = true;
        }
    }
    if (count != 1 || dot == false)
        return (INVALID);

    int i = 0;
    if (sign_check(str) == true)
        i++;
    else if (sign_check(str) == false)
        return (INVALID);
    while (str[i])
    {
        if (!std::isdigit(str[i]) && str[i] != '.')
            return (INVALID);
        if (str[i] == '.')
            break ;
        i++;
    }
    if (str[i] == '.')
        i++;
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
}

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

static int invalid_chars(const std::string& literal)
{
    int i = 0;
    const char *str = literal.c_str();
    while (str[i])
    {
        if (!std::isdigit(str[i]) && str[i] != 'f'
        && str[i] != '.' && str[i] != '+' && str[i] != '-')
            return (INVALID);
        i++;
    }
    return (1);
}

static int detection(const std::string& literal)
{
    if (literal.empty())
        return (INVALID);
    if (literal.length() == 1 && std::isalpha(literal.c_str()[0]))
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

void ScalarConverter::convert(const std::string& literal){
    int type = detection(literal);
    if (type == CHAR)
        std::cout << "char detected\n";
    else if (type == INT)
        std::cout << "int detected\n";
    else if (type == FLOAT)
        std::cout << "float detected\n";
    else if (type == DOUBLE)
        std::cout << "double detected\n";
    else
        std::cout << "invalid\n";
}


// reminder handle .5f .5 .f double and float. and check edge case ...

/* maybe introduce a boolean
bool beforeDigit = false;

while (str[i])
{
    if (!std::isdigit(str[i]) && str[i] != '.')
        return (INVALID);

    if (std::isdigit(str[i]))
        beforeDigit = true;

    if (str[i] == '.')
        break;

    i++;
}

if (!beforeDigit)
    return (INVALID);
*/

// float_double accepts multiple fs 42.0ff

/*

// fix by std::isprint(literal[0]) && !std::isdigit(literal[0]), is pritable instead of is alpha.
The subject says

one printable non-digit character

not

alphabetic character.

So these should all be chars:

*
@
%
?
:
$
#
!
~
*/




/*

Instead of

if (literal.length() == 1 && std::isalpha(literal.c_str()[0]))

make it

if (literal.length() == 1
    && std::isprint(literal[0])
    && !std::isdigit(literal[0]))
    return (CHAR);

*/