mydc:mydc.o
	gcc209 -m32 -march=i386 $< -g -o $@
mydc.o:mydc.s
	as --32 -march=i386 $< -o $@

test:test.o
	gcc209 -m32 -march=i386 $< -g -o $@
test.o:test.s
	as --32 -march=i386 $< -o $@

clear:
	rm -f mydc.o mydc
cleartest:
	rm -f test.o test
