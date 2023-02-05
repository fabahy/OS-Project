#include "syscall.h"

int main(){
    PrintString("Introduce our team:\n");
    PrintString("20120295 - Ngo Vo Quang Huy\n");
    PrintString("20120298 - Pham Bao Huy\n");
    PrintString("20120326 - Phan Phong Luu\n\n");
    PrintString(
        "ASCII: at directory code, run ./userprog/nachos -rs 1023 -x ./test/ascii to "
        "print the ASCII table\n");
    PrintString(
        "Sort: at directory code, run ./userprog/nachos -rs 1023 -x ./test/bubblesort to "
        "start the sort program\n");
    PrintString("\t- Enter n (the length of the array, 0 < n <= 100)\n");
    PrintString("\t- Enter n elements of the array\n");
    PrintString("\t- The program will print out the sorted array and then exit\n");
    Halt();
}