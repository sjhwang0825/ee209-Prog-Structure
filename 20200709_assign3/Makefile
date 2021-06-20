CC=gcc
base: testclient1 testclient2
array: testclient1
hash: testclient2

testclient1: testclient.o customer_manager1.o
	$(CC) $^ -o testclient1
testclient2: testclient.o customer_manager2.o
	$(CC) $^ -o testclient2

testclient.o: testclient.c customer_manager.h
	$(CC) -c $<
customer_manager1.o: customer_manager1.c customer_manager.h
	$(CC) -c $<
customer_manager2.o: customer_manager2.c customer_manager.h
	$(CC) -c $<

clean:
	rm -f *.o testclient1 testclient2
