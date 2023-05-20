CC:=g++
MAIN:=project
OUT:=project
ACCPETED:=accept.txt
ERROR:=error.txt

all: clean compile

clean:
	@rm -f *.o *.exe *.out

compile:
	@$(CC) $(MAIN).cpp -o $(OUT)

test_accept:
	@./$(OUT) $(ACCPETED)

test_error:
	@./$(OUT) $(ERROR)