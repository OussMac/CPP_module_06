#include "ScalarConverter.hpp"
#include "convert_utils.hpp"
#include "preparser.hpp"

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
    if (str[0] == '.' && std::isdigit(str[1]))
        return (true);
    if ((str[0] == '-' || str[0] == '+') && (std::isdigit(str[1]) || str[1] == '.'))
        return (true);
    return (false);
}

static int dot_position(const char *str)
{
    int i = 0;
    while (str[i])
    {
        if (str[i] == '.')
            return (i);
        i++;
    }
    return (-1);
}

static int right_left(const char *str, int pos)
{
    if (pos == 0)
        return (LEFT);
    else if (pos == 1)
    {
        if ((str[0] == '-' || str[0] == '+'))
            return (LEFT);
        else
        {
            if (str[pos + 1] == 'f' || str[pos + 1] == '\0')
                return (RIGHT);
            return (0);
        }
    }
    else if (pos == static_cast<int>(std::strlen(str) - 1))
    {
        if (str[pos + 1] == '\0' || str[pos + 1] == 'f')
        {
            if (str[pos + 1] == 'f' && str[pos + 2] != '\0')
                return (0);
            return (RIGHT);
        }
    }
    else if (pos == static_cast<int>(std::strlen(str) - 2))
    {
        if (str[pos + 1] == 'f' && str[pos + 2] != '\0')
            return (0);
        return (RIGHT);
    }
    return (0);
}

static int valid_right(const char *str, int pos)
{
    int suffix_pos = pos + 1;
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

static int valid_left(const char *str, int pos)
{
    bool float_found = false;
    pos++;
    if (str[pos] == '\0' || str[pos] == 'f')
        return (0);
    while (str[pos])
    {
        if (str[pos] != 'f' && !std::isdigit(str[pos]))
            return (0);
        if (str[pos] == 'f')
            float_found = true;
        pos++;
    }
    if (float_found)
        return (FLOAT);
    return (DOUBLE);
}

static int digitNextToDot(const char *str)
{
    int dot_pos = dot_position(str);
    if (dot_pos == -1)
        return (0);

    int check_place =  right_left(str, dot_pos);
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

static int float_double(const std::string& literal)
{
    bool dot = false;
    int count = 0;
    int fcount = 0;
    const char *str = literal.c_str();

    for (int i = 0; str[i]; i++)
    {
        if (str[i] == '.')
        {
            count++;
            dot = true;
        }
        if (str[i] == 'f')
            fcount++;
    }

    if (fcount > 1 || count != 1 || dot == false)
        return (INVALID);
    if (fcount == 1)
    {
        if (after_f(str))
            return (INVALID);
    }

    int i = 0;
    if (sign_check(str) == true)
        i++;
    else if (sign_check(str) == false)
        return (INVALID);
    
    int dntd = digitNextToDot(str);
    if (dntd)
        return (dntd);

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

void ScalarConverter::convert(const std::string& literal){
    int type = detection(literal);
    type = preparser_bounds(literal, type);
    if (type == CHAR)
        convertChar(literal);
    else if (type == INT)
        convertInt(literal);
    else if (type == FLOAT)
        convertFloat(literal);
    else if (type == DOUBLE)
        convertDouble(literal);
    else
        std::cout << "invalid\n";
}
