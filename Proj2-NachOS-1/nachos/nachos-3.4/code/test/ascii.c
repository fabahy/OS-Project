#include "syscall.h"

int main() {
    int i;
    PrintString("\t============== ASCII table ==============\n");
    for (i = 32; i <= 126; i++) {
        PrintInt(i);
        PrintString(" ");
        PrintChar((char)i);
        PrintString("\n");
    }
    Halt();
}