all: 
	gcc fork.c -o fork
	gcc select.c -o select
clean:
	rm fork
	rm select
