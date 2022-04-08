-all: client-server

client-server:
	clear
	gcc -c client.c admin.c cal.c
	gcc admin.c -lpthread -o admin.exe
	gcc client.o -o client.exe
	gcc cal.o -o cal.exe
	./admin.exe