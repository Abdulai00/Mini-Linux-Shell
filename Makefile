

d8sh:
			gcc -o d8sh d8sh.c executor.c lexer.c parser.tab.c -lreadline

clean:
	rm rf *.c
