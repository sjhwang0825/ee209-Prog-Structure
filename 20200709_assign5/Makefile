ish:ish.o dynarray.o dynarray.h
	gcc209 -D_DEFAULT_SOURCE -D_GNU_SOURCE dynarray.o ish.o -g -o ish

dynarray.o:dynarray.c dynarray.h
	gcc209 -c dynarray.c

ish.o:ish.c dynarray.h
	gcc209 -c ish.c

clean:
	rm -f *.o
