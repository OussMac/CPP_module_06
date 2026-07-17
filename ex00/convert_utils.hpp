#ifndef CONVERT_UTILS_HPP
#define CONVERT_UTILS_HPP

#include <iostream>
#include <iomanip> // fixed, precesion
#include <cstdlib> // strtoX
#include <limits> // limits
#include <cmath> // isnan,isinf
#include <cctype> // isdigit, isprint


void convertChar(const std::string& literal);
void convertInt(const std::string& literal);
void convertFloat(const std::string& literal);
void convertDouble(const std::string& literal);

#endif // CONVERT_UTILS_HPP
