hw3: hw3.o
	g++ hw3.o -o hw3
hw3.o: hw3.cc product_record.h
clean: 
	rm -f *.o
	rm -f part2
	rm -f hw1
