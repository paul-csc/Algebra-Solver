#include "utils.h"
#include <format>
#include <iostream>

ArenaAllocator::ArenaAllocator(size_t chunkSize) : m_Size(chunkSize) {
    m_Buffer = new std::byte[m_Size];
    m_Offset = m_Buffer;
}

ArenaAllocator::~ArenaAllocator() {
    delete[] m_Buffer;
}

[[noreturn]] void Error(const std::string& msg) {
    std::cerr << "Error: " << msg << "\n";
    std::cin.get();
    std::exit(1);
}
