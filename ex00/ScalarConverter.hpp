#ifndef SCALARCONVERTER_HPP
#define SCALARCONVERTER_HPP

#include <iostream>

# define CHAR 1
# define INT 2
# define FLOAT 3
# define DOUBLE 4
# define INVALID -1

class ScalarConverter {
    private:
    // OCF, non instantiable.
    ScalarConverter();
    ScalarConverter(const ScalarConverter& other);
    ScalarConverter& operator=(const ScalarConverter& other);
    ~ScalarConverter();

    public:
    static void convert(const std::string& literal);
};

#endif // SCALARCONVERTER_HPP
