wakeonlan: participants.o wakeonlan.c discovery.o
	gcc participants.o discovery.o wakeonlan.c -o wakeonlan  -lpthread

participants.o: participants.c
	gcc -c participants.c

discovery.o: discovery.c
	gcc -c discovery.c

clean:
	rm participants.o discovery.o