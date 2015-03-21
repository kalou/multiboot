CFLAGS += -Wall -Werror -nostdlib -fno-stack-protector \
 -fno-builtin -I. -I../include -msse2

.S.o:
	$(CC) -c -o $@ $(CFLAGS) $<
.c.o:
	$(CC) -c -o $@ $(CFLAGS) $<

$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

all: $(OBJS) $(LIB)
	@(echo done)

clean:
	rm -f $(OBJS) $(LIB)
