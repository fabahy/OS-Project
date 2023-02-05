#include "syscall.h"

int main() {
    char c;
    PrintString("Enter your char: ");
    c = ReadChar();
    PrintString("Your char: ");
    PrintChar(c);
    PrintString("\n");
    Halt();
}