#include "ScalarConverter.hpp"
#include "preparser.hpp"
#include "convert_utils.hpp"

static int checkIntBounds(const std::string& literal)
{
    std::string number = literal;
    bool negative = false;

    if (number[0] == '+' || number[0] == '-')
    {
        negative = (number[0] == '-');
        number.erase(0, 1);
    }

    // Remove leading zeros
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
        // Compare magnitude against INT_MIN
        const std::string min = "2147483648";

        if (number.length() > min.length())
            return INVALID;

        if (number.length() == min.length() && number > min)
            return INVALID;
    }

    return INT;
}

static int checkFloatBounds(const std::string& literal)
{
    float value = std::strtof(literal.c_str(), NULL);

    if (std::isinf(value))
        return INVALID;

    return FLOAT;
}

static int checkDoubleBounds(const std::string& literal)
{
    double value = std::strtod(literal.c_str(), NULL);

    if (std::isinf(value))
        return INVALID;

    return DOUBLE;
}

int preparser_bounds(const std::string& literal, int type)
{
    if (type == INT)
        return checkIntBounds(literal);

    if (type == FLOAT)
        return checkFloatBounds(literal);

    if (type == DOUBLE)
        return checkDoubleBounds(literal);

    return type;
}


/*

Option 1 (Recommended): Don't reject float/double overflow

I would simply remove the float/double overflow checks from the preparser.

static int checkFloatBounds(const std::string&)
{
    return FLOAT;
}

static int checkDoubleBounds(const std::string&)
{
    return DOUBLE;
}

Then let

float value = std::strtof(...);
double value = std::strtod(...);



Option 2: Keep the check, but don't make it INVALID

If you really want to detect overflow beforehand, introduce another return value, for example

#define FLOAT_OVERFLOW 5
#define DOUBLE_OVERFLOW 6

and handle those specially in convert().

But that's more code and complexity for little benefit.

*/