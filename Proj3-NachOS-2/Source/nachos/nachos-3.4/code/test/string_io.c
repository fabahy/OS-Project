#include "syscall.h"

int main() {
    char str[30];
    PrintString("Enter your string: ");
    ReadString(str, 30);
    PrintString("Your string: ");
    PrintString(str);
    PrintString("\n");
    Halt();
}