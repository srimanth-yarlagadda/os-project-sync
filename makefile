-all: client-server

client-server:
	clear
	gcc -c client.c admin.c cal.c
	gcc admin.o cal.o -lpthread -o admin.exe
	gcc client.o -o client.exe
	./admin.exe