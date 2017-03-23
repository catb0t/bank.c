DEBUG_OPTS := -Wall -Wextra -Wfloat-equal -Winline -Wundef -Werror -fverbose-asm -Wint-to-pointer-cast -Wshadow -Wpointer-arith -Wcast-align  -Wcast-qual -Wunreachable-code -Wstrict-overflow=5 -Wwrite-strings -Wconversion --pedantic-errors -ggdb -Wredundant-decls -Werror=maybe-uninitialized

MEM_OPTS := -fstack-protector -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer

ifeq ($(CCPLUS), 1)
	OPTS += -std=c++14
	CC := c++
else
	OPTS += -std=gnu11
	DEBUG_OPTS += -Wstrict-prototypes -Wmissing-prototypes
endif

ifeq ($(CC), gcc)
  #DEBUG_OPTS += -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn
  MEM_OPTS += -static-libasan -static-libtsan -static-liblsan -static-libubsan -lasan -lubsan
endif
ifeq ($(CC), cc)
  #DEBUG_OPTS += -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn
  MEM_OPTS += -static-libasan -static-libtsan -static-liblsan -static-libubsan -lasan -lubsan
endif

normal: bt.c
	$(CC) $(DEBUG_OPTS) bt.c -o bt
	
mem: bt.c
	$(CC) $(DEBUG_OPTS) $(MEM_OPTS) bt.c -o bt

test: test_bank.c
	$(CC) $(DEBUG_OPTS) test_bank.c -lcriterion -o test_bank
