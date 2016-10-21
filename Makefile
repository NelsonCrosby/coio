
CFLAGS ?= -Wall -Werror -MMD

pkgs = libuv lua52

CFLAGS += -fPIC
CFLAGS += $(shell pkg-config --cflags $(pkgs))
LDLIBS += $(shell pkg-config --libs $(pkgs))

coio = src/coio.o src/loop.o

all: coio.so
clean:
	rm -f \
	coio.so $(coio) $(coio:.o=.d)
.PHONY: all clean

coio.so: $(coio)
	$(CC) -shared $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(coio:.o=.d)
