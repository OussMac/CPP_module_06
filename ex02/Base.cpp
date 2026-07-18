#include "Base.hpp"
#include "A.hpp"
#include "B.hpp"
#include "C.hpp"

Base::~Base() {}

Base* generate() {
    int randomValue = std::rand() % 3;
    try {
        if (randomValue == 0)
            return new A();
        else if (randomValue == 1)
            return new B();
        else
            return new C();
    }
    catch (const std::bad_alloc& e) {
        std::cout << "Memory allocation failed: " << e.what() << std::endl;
        return NULL;
    }
}

void identify(Base* p) {
    if (dynamic_cast<A*>(p))
        std::cout << "A\n";
    else if (dynamic_cast<B*>(p))
        std::cout << "B\n";
    else if (dynamic_cast<C*>(p))
        std::cout << "C\n";
    else
        std::cout << "Unknown type\n";
}   

void identify(Base& p) {
    try {
        A& aRef = dynamic_cast<A&>(p);
        std::cout << "A\n";
        (void)aRef;
    } catch (const std::exception& e) {
        try {
            B& bRef = dynamic_cast<B&>(p);
            std::cout << "B\n";
            (void)bRef;
        } catch (const std::exception& e) {
            try {
                C& cRef = dynamic_cast<C&>(p);
                std::cout << "C\n";
                (void)cRef;
            } catch (const std::exception& e) {
                std::cout << "Unknown type\n";
            }
        }
    }
}