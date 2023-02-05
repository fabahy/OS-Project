#include "syscall.h"

int main() {
    int n;
    PrintString("Enter your number: ");
    n = ReadInt();
    PrintString("Your number: ");
    PrintInt(n);
    PrintString("\n");
    Halt();
}