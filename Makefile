wakeonlan: participants.o wakeonlan.c discovery.o monitoring.o interface.o election.o
	gcc participants.o discovery.o monitoring.o interface.o election.o wakeonlan.c -o  sleep_server  -lpthread

participants.o: participants.c
	gcc -c participants.c

discovery.o: discovery.c
	gcc -c discovery.c

monitoring.o: monitoring.c
	gcc -c monitoring.c

interface.o: interface.c
	gcc -c interface.c

election.o: election.c
	gcc -c election.c

clean:
	rm participants.o discovery.o monitoring.o interface.o election.o