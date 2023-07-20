wakeonlan: participants.o wakeonlan.c discovery.o monitoring.o
	gcc participants.o discovery.o monitoring.o wakeonlan.c -o  wakeonlan  -lpthread

participants.o: participants.c
	gcc -c participants.c

discovery.o: discovery.c
	gcc -c discovery.c

monitoring.o: monitoring.c
	gcc -c monitoring.c
clean:
	rm participants.o discovery.o monitoring.o