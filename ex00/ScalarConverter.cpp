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

/*
char    : exactly one character, and that character is NOT a digit
            e.g. "c", "a", "Z"

int     : optional leading '-', then only digits, no '.'
            e.g. "42", "-42", "0"

float   : digits + one '.' + digits, ending in 'f'
          OR one of: "-inff", "+inff", "nanf"
            e.g. "4.2f", "-4.2f", "0.0f"

double  : digits + one '.' + digits, NO trailing 'f'
          OR one of: "-inf", "+inf", "nan"
            e.g. "4.2", "-4.2", "0.0"
*/

static bool sign_check(const char *str)
{
    if (std::isalpha(str[0]))
        return (true);
    else if ((str[0] == '-' || str[0] == '+') && std::isdigit(str[1]))
        return (true);
    return (false);
}

static int float_double(const std::string& literal)
{
    bool dot = false;
    int count = 0;
    const char *str = literal.c_str();

    for(int i = 0; str[i]; i++)
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
    // sign check, and skip first digit or sign.
    if (sign_check(str) == true)
        i++;
    else if (sign_check(str) == false)
        return (INVALID);
    // digits before .
    while (str[i])
    {
        if (!std::isdigit(str[i] && str[i] != '.'))
            return (INVALID);
        if (str[i] == '.')
            break ;
        i++;
    }
    if (str[i] == '.')
        i++;
    // digits after .
    while (str[i])
    {
        if (!std::isdigit(str[i]) && str[i] != 'f')
            return (INVALID);
        if (str[i] == 'f')
            break ;
        i++;
    }
    // trailing f or null.
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
    if (literal == "-inf" || literal == "-inff"
        || literal == "+inf" || literal == "+inff"
        || literal == "nan" || literal == "nanf")
        return (1);
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
    // empty edge case.
    if (literal.empty())
        return (INVALID);
    // check char.
    if (literal.length() == 1 && std::isalpha(literal.c_str()[0]))
        return (CHAR);

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



