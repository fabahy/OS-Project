/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

 

int main()
{
    int A[100]; /* size of physical memory; with code, we'll run out of space!*/
    int i, j, k, n, tmp, option;

    PrintString("\t============== Bubble sort ==============\n");
    PrintString("1. Ascending order\n");
    PrintString("2. Descending order\n");
    PrintString("Your order: ");

    option = ReadInt();
    while (option != 1 && option != 2) {
        PrintString("Illegal input, try again: ");
        option = ReadInt();
    }

    PrintString("Enter number of elemets N (1-100): ");
    n = ReadInt();

    while (n > 100 || n <= 0) {
        PrintString("Illegal input, try again: ");
        n = ReadInt();
    }

    PrintString("Enter N integers, separate by space or newline.\n");

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < n; i++) {
        PrintString("A[");
        PrintInt(i);
        PrintString("] = ");
        A[i] = ReadInt();
    }
    PrintString("\nThe original list is:\n");
    PrintString("[ ");
    PrintInt(A[0]);
    for (k = 1; k < n; k++)
    {
        PrintString(", ");
        PrintInt(A[k]);
    }
    PrintString(" ]\n");
    /* then sort! */
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (A[j] > A[j + 1])
            { /* out of order -> need to swap ! */
                tmp = A[j];
                A[j] = A[j + 1];
                A[j + 1] = tmp;
            }
        }  
    }
    

    if (option == 1) {
        PrintString("The sorted list in ascending is:\n");
        PrintString("[ ");
        PrintInt(A[0]);
        for (k = 1; k < n; k++)
        {
            PrintString(", ");
            PrintInt(A[k]);
        }
        PrintString(" ]\n");
    }
    else {
        PrintString("The sorted list in descending is:\n");
        PrintString("[ ");
        PrintInt(A[n-1]);
        for (k = n-2; k >= 0; k--)
        {
            PrintString(", ");
            PrintInt(A[k]);
        }
        PrintString(" ]\n");
    }
    
    Halt();
    //Exit(A[0]); /* and then we're done -- should be 0! */
}