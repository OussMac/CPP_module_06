#include "convert_utils.hpp"

static void printChar(char c)
{
    if (!std::isprint(static_cast<unsigned char>(c)))
        std::cout << "char: Non displayable\n";
    else
        std::cout << "char: '" << c << "'\n";
}

static void printInt(int value)
{
    std::cout << "int: " << value << '\n';
}

static void printFloat(float value)
{
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "float: " << value << "f\n";
}

static void printDouble(double value)
{
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "double: " << value << '\n';
}

void convertChar(const std::string& literal)
{
    char value = literal[0];

    printChar(value);
    printInt(static_cast<int>(value));
    printFloat(static_cast<float>(value));
    printDouble(static_cast<double>(value));
}

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
        if (value < std::numeric_limits<char>::min()
            || value > std::numeric_limits<char>::max())
            std::cout << "char: impossible\n";
        else
            printChar(static_cast<char>(value));

        if (value < std::numeric_limits<int>::min()
            || value > std::numeric_limits<int>::max())
            std::cout << "int: impossible\n";
        else
            printInt(static_cast<int>(value));
    }

    printFloat(value);
    printDouble(static_cast<double>(value));
}

void convertDouble(const std::string& literal)
{
    double value = std::strtod(literal.c_str(), NULL);

    if (std::isnan(value) || std::isinf(value))
    {
        std::cout << "char: impossible\n";
        std::cout << "int: impossible\n";
    }
    else
    {
        if (value < std::numeric_limits<char>::min()
            || value > std::numeric_limits<char>::max())
            std::cout << "char: impossible\n";
        else
            printChar(static_cast<char>(value));

        if (value < std::numeric_limits<int>::min()
            || value > std::numeric_limits<int>::max())
            std::cout << "int: impossible\n";
        else
            printInt(static_cast<int>(value));
    }

    printFloat(static_cast<float>(value));
    printDouble(value);
}