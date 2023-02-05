// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"


#define INT32_MAX 2147483647	
#define INT32_MIN -2147483648LL

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

char* User2System(int virtAddr, int limit) {
	int i;
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];
	if (kernelBuf == NULL)
		return kernelBuf;
		
	memset(kernelBuf, 0, limit + 1);
	
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr, int len, char* buffer) {
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

void IncreasePC() {
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    counter = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		printf("No valid translation found");
		interrupt->Halt();
		break;
	case ReadOnlyException:
		DEBUG('a', "Write attempted to page marked \"read-only\".\n");
		printf("Write attempted to page marked \"read-only\".");
		interrupt->Halt();
		break;
	case BusErrorException:
		DEBUG('a', "Translation resulted in an invalid physical address.\n");
		printf("Translation resulted in an invalid physical address.");
		interrupt->Halt();
		break;
	case AddressErrorException:
		DEBUG('a', "Unaligned reference or one that was beyond the end of the address space.\n");
		printf("Unaligned reference or one that was beyond the end of the address space.");
		interrupt->Halt();
		break;
	case OverflowException:
		DEBUG('a', "Integer overflow in add or sub.\n");
		printf("Integer overflow in add or sub.");
		interrupt->Halt();
		break;
	case IllegalInstrException:
		DEBUG('a', "Unimplemented or reserved instr.\n");
		printf("Unimplemented or reserved instr.");
		interrupt->Halt();
		break;
	case NumExceptionTypes:
		DEBUG('a', "Number exception types.\n");
		printf("Number exception types.");
		interrupt->Halt();
		break;
	case SyscallException:
		switch (type)
		{
			case SC_Halt:
				DEBUG('a', "Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;
			case SC_ReadInt: {
				// Input: Khong co
				// Output: Tra ve so nguyen doc duoc tu man hinh console.
				// Chuc nang: Doc so nguyen tu man hinh console.
				char* buffer;
				int MAX = 256;
				buffer = new char[MAX + 1];

				/* Doc so nguyen nhap tu console toi da MAX ki tu, sau do tra ve so ki tu doc duoc */
				int numOfChar = gSynchConsole->Read(buffer, MAX);
				
				int result = 0;
				bool isNeg = false;
				int first = 0;
				int last = 0;

				/* Kiem tra so luong chu so vuot qua so nguyen 32 bit */
				if (numOfChar > 15) {
					machine->WriteRegister(2, 0);
					delete buffer;
					IncreasePC();
					return;
				}     

				/* Kiem tra so nguyen am hay duong */
				if(buffer[0] == '-') {
					isNeg = true;
					first = 1;
					last = 1;                        			   		
				}
				for(int i = first; i < numOfChar; i++) {
					/* Kiem tra so nguyen hop le duoi dang so thuc 
					Vi du: 9.0000 => 9 */
					if(buffer[i] == '.') {
						int j = i + 1;
						for(; j < numOfChar; j++) {
							if (buffer[j] != '0') {
								machine->WriteRegister(2, result);
								delete buffer;
								IncreasePC();
								return;
							}
						}
						last = i - 1;				
						break;                           
					}
					/* Kiem tra ton tai ki tu khong phai la so */
					else if (buffer[i] < '0' || buffer[i] > '9') {
						machine->WriteRegister(2, result);
						delete buffer;
						IncreasePC();
						return;
					}
					last = i;    
				}			
				/* Chuyen chuoi buffer doc tu console ve so nguyen 32 bit */
				long long int temp = 0;
				for(int i = first; i <= last; i++) {
					temp = (temp * 10) + (int)(buffer[i] - '0');
				}
				if (isNeg) {
					temp = temp * (-1);
				}
				/* Kiem tra gioi han duoi cua so nguyen 32 bit */
				if (temp < INT32_MIN) {
					machine->WriteRegister(2, result);
					delete buffer;
					IncreasePC();
					return;
				}
				/* Kiem tra gioi han tren cua so nguyen 32 bit */
				if (temp > INT32_MAX) {
					machine->WriteRegister(2, result);
					delete buffer;
					IncreasePC();
					return;
				}
				/* Sau khi kiem tra, so nguyen hop le duoc ghi vao thanh ghi */
				result = temp;
				machine->WriteRegister(2, result);
				delete buffer;
				break;
			}
			case SC_PrintInt: {
				// Input: So nguyen integer 32 bit
				// Output: Khong co
				// Chuc nang: In so nguyen 32 bit len man hinh console
				long long int number = machine->ReadRegister(4);

				char* buffer;
				int MAX = 256;
				buffer = new char[MAX + 1];
				
				/* TH: La so 0 */
				if (number == 0) {
					gSynchConsole->Write("0", 1); 
					IncreasePC();
					return;    
				}
				bool isNeg = false;
				int numOfChar = 0;
				int first = 0; 
		
				if(number < 0) {
					isNeg = true;
					number = number * -1; // Nham chuyen so am thanh so duong de tinh so chu so
					first = 1; 
				} 	

				/* Dem so chu so cua so nguyen da doc duoc */
				int temp = number;
				while (temp != 0) {
					numOfChar++;
					temp /= 10;
				}

				/* Chuyen so nguyen 32 bit doc tu console ve dang chuoi buffer */
				for(int i = first + numOfChar - 1; i >= first; i--) {
					buffer[i] = (char)((number % 10) + 48);
					number /= 10;
				}

				/* TH: La so nguyen am, sau do ghi chuoi buffer len man hinh console */
				if (isNeg) {
					buffer[0] = '-';
					buffer[numOfChar + 1] = '\0';
					gSynchConsole->Write(buffer, numOfChar + 1);
					delete buffer;
					IncreasePC();
					return;
				}

				buffer[numOfChar] = '\0';	
				/* Goi ham Write cua lop SynchConsole de in ki tu */
				gSynchConsole->Write(buffer, numOfChar);
				delete buffer;
				break;
			}
			case SC_ReadChar: {
				// Input: Khong co
				// Output: Tra ve 1 ki tu doc duoc tu man hinh console.
				// Chuc nang: Doc 1 ki tu tu man hinh console
				int MAX = 256;
				char* buffer = new char[MAX + 1];
				int numOfChar = gSynchConsole->Read(buffer, MAX);

				char c = buffer[0]; // Note: Neu nhap hon 1 ki tu thi chi doc 1 ki tu dau tien
				/* Ghi 1 ki tu vao thanh ghi */ 
				machine->WriteRegister(2, c);
				delete[] buffer;
				break;
			}
			case SC_PrintChar: {
				// Input: 1 Ki tu
				// Output: Khong co
				// Chuc nang: In 1 ki tu len man hinh console

				/* Doc 1 ki tu tu thanh ghi */
				char c = (char)machine->ReadRegister(4);
				/* Goi ham Write cua lop SynchConsole de in ki tu */
				gSynchConsole->Write(&c, 1);
				break;
			}
			case SC_ReadString: {
				// Input: Chuoi buffer kieu char[] va do dai toi da cua chuoi nhap vao
				// Output: Khong co
				// Chuc nang: Doc mot chuoi ki tu voi buffer va do dai cua chuoi
				int virtAddr, length;
				char* buffer;

				/* Lay dia chi chuoi da doc duoc tu thanh ghi so 4 */
				virtAddr = machine->ReadRegister(4);
				/* Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5 */
				length = machine->ReadRegister(5);

				/* Copy dia chi chuoi tu vung nho User Space sang System Space */
				buffer = User2System(virtAddr, length);  

				/* Goi ham Read cua lop SynchConsole de doc chuoi */
				gSynchConsole->Read(buffer, length);

				/* Copy dia chi chuoi tu vung nho System Space sang User Space */
				System2User(virtAddr, length, buffer);
				delete buffer; 
				break;
			}
			case SC_PrintString: {
				// Input: Chuoi buffer kieu char[]
				// Output: Chuoi doc duoc tu buffer(char[])
				// Chuc nang: In mot chuoi ki tu ra man hinh console
				int virtAddr;
				char* buffer;
				int length = 0;

				/* Lay dia chi chuoi da doc duoc tu thanh ghi so 4 */
				virtAddr = machine->ReadRegister(4);
				
				/* Copy dia chi chuoi tu vung nho User Space sang System Space*/
				buffer = User2System(virtAddr, 255);
				
				/* Dem do dai that su cua chuoi */
				while (buffer[length] != '\0') {
					length++;
				}
					 
				/* Goi ham Write cua lop SynchConsole de in chuoi */
				gSynchConsole->Write(buffer, length + 1);
				delete buffer;
				break;
			}
			default:
				printf("Unexpected user mode exception %d %d\n", which, type);
				ASSERT(FALSE);
				break;
		}
		IncreasePC();
	}
}
