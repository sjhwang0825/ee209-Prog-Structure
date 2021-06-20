##20200709_SungJoonHwang
##assignment 4

### --------------------------------------------------------------------
### mydc.s
###
### Desk Calculator (dc)
### --------------------------------------------------------------------

	.equ   ARRAYSIZE, 20
	.equ   EOF, -1
    .equ   SIZE, 4
	
.section ".rodata"

##print integer in top of stack
intoutput:
    .asciz "%d\n"

##print error
ErrorOutput:
    .asciz "dc: stack empty\n"

DigitInputError:
    .asciz "Unacceptable digit Input is removed : %s\n"

OperationInputError:
    .asciz "Unaccptable operation Input is removed : %s\n"
MismatchInputError:
    .asciz "Mismatched operation : %s\n"

scanfFormat:
	.asciz "%s"
### --------------------------------------------------------------------

        .section ".data"

### --------------------------------------------------------------------

        .section ".bss"
buffer:
        .skip  ARRAYSIZE

### --------------------------------------------------------------------

	.section ".text"

	## -------------------------------------------------------------
	## int main(void)
	## Runs desk calculator program.  Returns 0.
	## -------------------------------------------------------------

	.globl  main
	.type   main,@function

main:

	pushl   %ebp
	movl    %esp, %ebp

    ##while loop
    loop:
    jmp input
    inputreturn:

    ##read current input
    movl $0, %ebx
    movb buffer, %bl

    cmpl $'_', %ebx
    je digitErrorControl

    pushl %ebx
    ##check operation is valid
    call isdigit
    addl $4, %esp
    cmpl $0, %eax
    je OperationErrorControl
    OperationErrorControlPass:

    ##reset ebx register again
    movl $0, %ebx
    movb buffer, %bl

    cmpl $'p', %ebx
    je pt
    cmpl $'q', %ebx
    je quit
    cmpl $'+', %ebx
    je sum
    cmpl $'-', %ebx
    je subtract
    cmpl $'*', %ebx
    je multiple
    cmpl $'/', %ebx
    je divide
    cmpl $'|', %ebx
    je ABSsum
    cmpl $'f', %ebx
    je printLifo
    cmpl $'c', %ebx
    je clear
    cmpl $'d', %ebx
    je duplicate
    cmpl $'r', %ebx
    je reverse

    ##check wether operation is matched or not
    pushl %ebx
    call isdigit
    addl $4, %esp
    cmpl $0, %eax
    je OperationMismatchError

    jmp digitErrorControl
    digitErrorControlPass:
    ##reset ebx register again
    movl $0, %ebx
    movb buffer, %bl

    cmpl $'_', %ebx
    je fixsign
    
    ##checking isdigit
    pushl %ebx
    call isdigit
    addl $SIZE, %esp
    cmpl $0, %eax
    jne digitInput

    ##input is neither integer and Operation
    ##jmp WrongInput
    
    ##goto whilelpop again
    jmp loop

##single character is regared as operation
##check wheter operation is matched or not
OperationMismatchError:
    pushl $buffer
    pushl $MismatchInputError
    call printf
    addl $8, %esp
    jmp loop

##remove wrong operation input
##such as ppp, pabcd
OperationErrorControl:
    ##the first letter is a char
    movl $0, %ebx
    movb buffer+1, %bl
    cmpl $0, %ebx
    ##operation should be a single letter
    je OperationErrorControlPass
    jmp WrongOperationInput

WrongOperationInput:
    pushl $buffer
    pushl $OperationInputError
    call printf
    addl $8, %esp
    jmp loop


##remove wrong integer input
##such as 123ABC, 1A etc
digitErrorControl:
    ##iterate the given input with ecx
    movl $0, %ecx
    
    DECLoop:
    ##current digit to ebx register
    movl $0, %ebx
    movb buffer(%ecx), %bl

    ##increase ecx by 1 for '_' char
    cmpl $'_', %ebx
    je IncEcx

    ##when 0(NULL)is reached, it's pure integer
    cmpl $0, %ebx
    je digitErrorControlPass

    ##call isdigit
    pushl %ebx
    call isdigit
    addl $4, %esp

    ##return value of isdigit
    cmpl $0, %eax
    ##it isn't pure integer
    je WrongDigitInput

    IncEcx:
    ##increment ecx by 1
    addl $1, %ecx
    jmp DECLoop

##print Error message for wrong input
WrongDigitInput:
    pushl $buffer
    pushl $DigitInputError
    call printf
    addl $8, %esp
    jmp loop

##reverse operation
reverse:
    ##eax=(distance btw ebp esp)/4
    movl $0, %edx
    movl $SIZE, %ecx
    movl %ebp, %eax
    subl %esp, %eax
    idivl %ecx


    negl %eax
    movl (%ebp,%eax,SIZE), %ebx
    movl SIZE(%ebp,%eax,SIZE), %ecx

    movl %ecx, (%ebp,%eax,SIZE)
    movl %ebx, SIZE(%ebp,%eax,SIZE)
    jmp loop

##duplicate operation
duplicate:
    pushl (%esp)
    jmp loop

##clear operation
clear:
    movl %ebp, %esp
    jmp loop

##printing stack in LIFO
printLifo:
    ##eax=(distance btw ebp esp)/4
    movl $0, %edx
    movl $SIZE, %ecx
    movl %ebp, %eax
    subl %esp, %eax
    idivl %ecx

    negl %eax
    pushl %eax

    printloop:
    movl (%esp), %eax
    cmp $0, %eax
    je printloopend

    pushl (%ebp, %eax, SIZE)
    pushl $intoutput
    call printf
    addl $8, %esp

    movl (%esp), %eax
    addl $1, %eax
    movl %eax, (%esp)

    jmp printloop

    printloopend:
    addl $4, %esp
    jmp loop

    

## the case that start with '_'
fixsign:
pushl $buffer+1
call atoi
add $SIZE, %esp
negl %eax
##save digit to stack
pushl %eax
jmp loop

##when input is digit
digitInput:
pushl $buffer
call atoi
add $SIZE, %esp
##save digit to stack
pushl %eax
jmp loop

pt:
cmpl %esp, %ebp
je ptError
pushl (%esp)
pushl $intoutput
call printf
add $8, %esp
jmp loop

ptError:
pushl $ErrorOutput
call printf
add $SIZE, %esp
jmp loop


##sum operation
sum:
cmpl %esp, %ebp
je stackEmptyError
popl %ecx
cmpl %esp, %ebp
je stackEmptyError2
popl %eax
addl %ecx, %eax
pushl %eax
jmp loop

stackEmptyError:
pushl $ErrorOutput
call printf
addl $4, %esp
jmp loop

stackEmptyError2:
pushl %ecx
pushl $ErrorOutput
call printf
add $4, %esp
jmp loop

##subtract operation
subtract:
cmpl %esp, %ebp
je stackEmptyError
popl %ecx
cmpl %esp, %ebp
je stackEmptyError2
popl %eax
subl %ecx, %eax
pushl %eax
jmp loop

##multiple operation
multiple:
cmpl %esp, %ebp
je stackEmptyError
popl %ecx
cmpl %esp, %ebp
je stackEmptyError2
popl %eax
imull %ecx
pushl %eax
jmp loop

##divide operation
divide:
cmpl %esp, %ebp
je stackEmptyError
popl %ecx
cmpl %esp, %ebp
je stackEmptyError2
popl %eax
##initialize edx register
movl $0, %edx
idivl %ecx
pushl %eax
jmp loop

##abssum operation
ABSsum:
    cmpl %esp, %ebp
    je stackEmptyError
    popl %ecx
    cmpl %esp, %ebp
    je stackEmptyError2
    popl %eax
    cmpl %ecx, %eax

    ##set eax is larger than ecx
    jl swap
    rtswap:
    pushl %eax
    pushl %ecx
    call func_abssum

    ##popping last 2 operand
    addl $8, %esp
    pushl %eax
    jmp loop


##function calling
func_abssum:
    ##prolog
    pushl %ebp
    movl %esp, %ebp

    ## -SIZE(%ebp) is for parameter sum
    subl $SIZE, %esp
    movl $0, (%esp)

    #current value is stored in ecx
    movl 8(%ebp), %ecx
    ##end value is stored in edx
    movl 12(%ebp), %edx
    absloop:
        cmp %ecx, %edx
        jl end
        movl %ecx, %ebx
        cmp $0, %ebx
        jge ABSend
        ##when i is negative, then make it postive
        ABS:
        negl %ebx
        ABSend:
        ##save it to sum parameter
        addl %ebx, -SIZE(%ebp)
        addl $1, %ecx
        jmp absloop
    end:
    ##save sum to eax register
    movl -SIZE(%ebp), %eax
    ##epilog
    movl %ebp, %esp
    popl %ebp
    ret


swap:
movl %ecx, %edx
movl %eax, %ecx
movl %edx, %eax
jmp rtswap


##standard input by scanf
input:
	## dc number stack initialized. %esp = %ebp
	
	## scanf("%s", buffer)
	pushl	$buffer
	pushl	$scanfFormat
	call    scanf
	addl    $8, %esp

	## check if user input EOF
	cmp	$EOF, %eax
	je	quit
    jmp inputreturn


quit:	
	## return 0
	movl    $0, %eax
	movl    %ebp, %esp
	popl    %ebp
	ret
