.code

GetPEB proc
	mov rax, gs:[60h] ; PEB
	ret
GetPEB endp

pebPatcher proc
	xor eax, eax
	call GetPEB
	movzx eax, byte ptr [rax+2h] ; PEB->BeingDebugged
	test eax, eax
	jnz PATCH
	ret

PATCH:
	xor eax, eax
	call GetPEB
	mov byte ptr [rax+2h], 0 ; PATCHED!
	ret
pebPatcher endp
end