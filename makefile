-all: client-server

client-server:
	gcc -c client.c admin.c
	gcc admin.c -o admin.exe
	gcc client.o -o client.exe
	clear
	./admin.exe