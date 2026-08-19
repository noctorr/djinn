#include "Program.hpp"

void vm::Program::initialise() noexcept
{
    Parser parser(text);

    VmResult result = parser.Parse();
    
}