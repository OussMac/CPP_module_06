#include "Base.hpp"

int main(void)
{
    std::srand(std::time(NULL));
    Base* basePtr = generate();

    std::cout << "Identifying using pointer:\n";
    identify(basePtr);

    std::cout << "\nIdentifying using reference:\n";
    identify(*basePtr);

    delete basePtr;
    return 0;
}