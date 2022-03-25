echo "***Shell Script Start***"
make
./cal.exe "cal.in.txt" > cal.out
./cal.exe "cal.in.txt" | fgrep $1