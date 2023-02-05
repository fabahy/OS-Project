/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int A[1024]; /* size of physical memory; with code, we'll run out of space!*/

int main()
{
    int i, j, k, n, count, tmp;

    PrintString("Nhap vao n (1-100): ");
    n = ReadInt();
    count = 0;

    while (n > 100 || n <= 0) {
        if (count >= 5) {
            PrintString("Ban da nhap sai qua 5 lan!!\n");
            //Exit(A[0]);
            return 1;
        }
        PrintString("Khong hop le, nhap lai n: ");
        n = ReadInt();
        count++;
    }

    PrintString("Nhap n so nguyen:\n");

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < n; i++)
        A[i] = ReadInt();

    /* then sort! */
    for (i = 0; i < n; i++)
        for (j = i; j < (n - i); j++)
            if (A[j] > A[j + 1])
            { /* out of order -> need to swap ! */
                tmp = A[j];
                A[j] = A[j + 1];
                A[j + 1] = tmp;
            }

    PrintString("Mang sau khi sap xep: ");
    for (k = 0; k < n; k++)
    {
        PrintInt(A[k]);
        PrintString(" ");
    }
    PrintString("\n");
    
    Halt();
    //Exit(A[0]); /* and then we're done -- should be 0! */
}