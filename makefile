CC = clang++
CFLAG = -std=c++11 -g -I. 
MYSQLFLAG = `mysql_config --cflags`

TARGET = main
LIBS = `mysql_config --libs`


OBJS = Database.o \
	   Tokenizer.o \
	   Parser.o \
	   IndexCon.o \
	   Utils.o \
	   main.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAG) -o $(TARGET) $(OBJS) $(LIBS)

Database.o: Database.cc
	$(CC) $(CFLAG) $(MYSQLFLAG) -o $@ -c $<

Utils.o: Utils.cc
	$(CC) $(CFLAG) $(MYSQLFLAG) -o $@ -c $<

Tokenizer.o: Tokenizer.cc
	$(CC) $(CFLAG) $(MYSQLFLAG) -o $@ -c $<

Parser.o: Parser.cc
	$(CC) $(CFLAG) $(MYSQLFLAG) -o $@ -c $<

IndexCon.o: IndexCon.cc
	$(CC) $(CFLAG) $(MYSQLFLAG) -o $@ -c $<

main.o: main.cpp
	$(CC) $(CFLAG) $(MYSQLFLAG) -o $@ -c $<


all:	
	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)