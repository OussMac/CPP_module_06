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
        const std::string min = "2147483648";

        if (number.length() > min.length())
            return INVALID;

        if (number.length() == min.length() && number > min)
            return INVALID;
    }
    return INT;
}

int preparser_bounds(const std::string& literal, int type)
{
    if (type == INT)
        return checkIntBounds(literal);
    return type;
}