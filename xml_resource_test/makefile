CC = gcc
CFLAGS = -g
CFLAGS += `xml2-config --cflags`
LDFLAGS = `xml2-config --libs`

SRCS = tres.c tres_test.c

test: $(SRCS)
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS)

clean:
	-rm *~ test test.exe
