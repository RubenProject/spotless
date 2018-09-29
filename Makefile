PROG = spotless
SOURCES = $(PROG).c sqlite3.c database.c template.c queue.c mongoose/mongoose.c
CFLAGS = -g -W -Wall $(CFLAGS_EXTRA) -DMG_ENABLE_HTTP_STREAMING_MULTIPART


ifeq ($(OS), Windows_NT)
  CFLAGS += -lws2_32
  CC = gcc
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S), Linux)
    CFLAGS += -ldl -lm -pthread
  endif
endif

all: $(PROG)

$(PROG): $(SOURCES)
	$(CC) $(SOURCES) -o $@ $(CFLAGS)

$(PROG).exe: $(SOURCES)
	cl $(SOURCES) /I/mongoose /MD /Fe$@

test: $(PROG)
	sh unit_test.sh $$(pwd)/$(PROG)

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.o a.out $(PROG)
