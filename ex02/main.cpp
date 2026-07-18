#include "Base.hpp"

int main(void)
{
    std::srand(std::time(NULL));
    Base* basePtr = generate();
    if (!basePtr) {
        std::cerr << "Failed to generate a Base object." << std::endl;
        return 1;
    }
    Base x = Base();

    std::cout << "Identifying using pointer:\n";
    identify(basePtr);

    std::cout << "\nIdentifying using reference:\n";
    identify(*basePtr);

    std::cout << "\nUnknown:\n";
    identify(x);

    delete basePtr;
    return 0;
}