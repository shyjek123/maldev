; WINDOWS 11, 23H2 22631

.data

EXTERN NtOpenProcessSSN:DWORD          
EXTERN mySyscall:QWORD

.code

NtOpenProcess PROC
    mov r10, rcx
    mov eax, NtOpenProcessSSN ; example of using a variable (using a var and func is better)
    jmp qword ptr [mySyscall] ; Jump to the specified address
    ret
NtOpenProcess ENDP



NtAllocateVirtualMemory PROC
		mov r10, rcx
		mov eax, 18h ; example of hard encoding
		jmp qword ptr [mySyscall]
		ret
NtAllocateVirtualMemory ENDP

NtWriteVirtualMemory PROC
		mov r10, rcx
		mov eax, 3Ah
		jmp qword ptr [mySyscall]
		ret
NtWriteVirtualMemory ENDP

NtCreateThreadEx PROC
		mov r10, rcx
		mov eax, 0C7h
		jmp qword ptr [mySyscall]
		ret
NtCreateThreadEx ENDP

NtWaitForSingleObject PROC
		mov r10, rcx
		mov eax, 4h
		jmp qword ptr [mySyscall]
		ret
NtWaitForSingleObject ENDP

NtClose PROC
		mov r10, rcx
		mov eax, 0Fh
		jmp qword ptr [mySyscall]
		ret
NtClose ENDP

end