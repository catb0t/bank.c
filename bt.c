#include "bank.h"

int main(void) {

  const char* const info[3] = {
    "cat",
    "cat",
    "Cat"
  };

  bankacct_t
    //*a = bankacct_ctor(NULL, 0),
    *b = bankacct_ctor(info, 1234);

  char* s = bankacct_see(b);

  printf("%s\n", s);

  safefree(s);

  bankacct_dtor(b);

  return 0;
}
