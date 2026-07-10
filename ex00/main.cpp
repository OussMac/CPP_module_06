#include "ScalarConverter.hpp"

int main (void)
{
    ScalarConverter::convert("c");
    ScalarConverter::convert("Z");
    ScalarConverter::convert("a");
    // int
    ScalarConverter::convert("42");
    ScalarConverter::convert("-01");
    ScalarConverter::convert("0000");

    // float
    ScalarConverter::convert("-4.2f");
    ScalarConverter::convert("0.0f");
    ScalarConverter::convert("-42.f");

    // double
    ScalarConverter::convert("0.0");
    ScalarConverter::convert("4.2");
    ScalarConverter::convert("105.500");
    ScalarConverter::convert("105.");

    ScalarConverter::convert("aa");
    return (0);
}