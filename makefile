-all: client-server

client-server:
	clear
	gcc -c client.c admin.c cal.c
	gcc admin.o cal.o -lpthread -o admin.exe
	gcc client.o -o client.exe
	./admin.exe

old:
	clear
	gcc -c client.c admin-old.c cal.c
	gcc admin-old.o cal.o -lpthread -o admin-old.exe
	gcc client.o -o client.exe
	./admin-old.exe