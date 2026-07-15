#include "Serializer.hpp"

int main(void)
{
    Data data;
    data.id = 42;
    data.name = "John Doe";
    data.score = 95.5;

    std::cout << "Original Data:\n";
    std::cout << "ID: " << data.id << "\n";
    std::cout << "Name: " << data.name << "\n";
    std::cout << "Score: " << data.score << "\n";

    uintptr_t serializedData = Serializer::serialize(&data);
    std::cout << "\nSerialized Data (uintptr_t): " << serializedData << "\n";

    Data* deserializedData = Serializer::deserialize(serializedData);
    std::cout << "\nDeserialized Data:\n";
    std::cout << "ID: " << deserializedData->id << "\n";
    std::cout << "Name: " << deserializedData->name << "\n";
    std::cout << "Score: " << deserializedData->score << "\n\n";

    std::cout << "Original pointer:     " << &data << std::endl;
    std::cout << "Deserialized pointer: " << deserializedData << std::endl;

    if (&data == deserializedData)
        std::cout << "Pointers are identical!" << std::endl;

    return 0;
}