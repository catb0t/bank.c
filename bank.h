#define CDTORDBG
#include "common.h"
#line __LINE__ "bank.h"
#define INITAL_WALLETS 3

typedef enum {
  ERR_NO_ERR,
  ERR_BAD_PIN,
  ERR_NO_STR,
  ERR_NULL_STRUCT,
  ERR_NUM_ERRTYPES
} bankerr_t;

// unordered unique user id
typedef uint64_t u3id_t;

typedef struct st_tsctn_t {

  // name of other participant
  char *other_name;

  // (optional) if the other ptcpnt has an account, this is it
  u3id_t other_id;

  // money
  uint64_t cred, debt;

  size_t uid;
} transactn_t;

typedef struct st_wallet_t {

  // wallet metadata
  char *name, *flags;

  // transaction hist
  transactn_t **creds, **debts;

  // id number (ordered)
  uint64_t num;

  // wallet-specific pin
  uint16_t pin;

  // can transactions be made?
  bool is_frozen;

  size_t uid;
} wallet_t;

typedef struct st_bankacct_t {
  // wallets list
  wallet_t** wallets;
  size_t     num_wallets;
  // convenience pointers to above
  wallet_t *sav, *chk, *fyz;

  // credentials
  char
    *uname, *pass,
    *realname;

  char*** info; // list of pointers

  // account pin
  uint16_t pin;

  // unordered account number
  u3id_t acct_u3id;

  uint64_t max_walletid;

  bool freeze_all, has_extra_wallets;

  size_t uid; // object uid; not memb-specific
} bankacct_t;


bankacct_t* bankacct_ctor (const char* const * const strdata, const uint16_t pin);
void        bankacct_dtor (bankacct_t* const bankacct);
char*        bankacct_see (const bankacct_t* const bact);

wallet_t* wallet_ctor (const char* name, const char* flags);
void      wallet_dtor (wallet_t* const wallet);
void     wallet_dtorn (wallet_t* * const ws, size_t len);

transactn_t transactn_ctor (const char* other_name, const u3id_t other, const uint64_t cred, const uint64_t debt);
void        transactn_dtor (const transactn_t* transactn);


void _bank_error (bankerr_t error, const bool fatal, const char* const file, const uint64_t line, const char* const func, const char* const fmt, ...);
#define bank_error(error, is_fatal, fmt, ...) _bank_error((error), (is_fatal), __FILE__, __LINE__, __func__, fmt, __VA_ARGS__)

void _bank_error (bankerr_t errt, const bool fatal, const char* const file, const uint64_t line, const char* const func, const char* const fmt, ...) {
  pfn();

  va_list vl;

  va_start(vl, fmt);

  size_t fmtlen = (safestrnlen(fmt) + 2) * 3;

  char* buf = (typeof(buf)) safemalloc(sizeof (char) * fmtlen);

  vsnprintf(buf, fmtlen, fmt, vl);

  va_end(vl);

  static const char* const errmsgs[] = {
    [ERR_NO_ERR]      = "No error occurred",
    [ERR_BAD_PIN]     = "PIN must be non-zero (given: 0)",
    [ERR_NO_STR]      = "found a NULL pointer or zero-length string where a valid string was needed",
    [ERR_NULL_STRUCT] = "dereferenced an object and found it to be unexpectedly null"
  };

  _Static_assert(
    (sizeof (errmsgs) / sizeof (char*) ) == ERR_NUM_ERRTYPES,
    "too many or too few error strings in bank_error"
  );

  // for embedded systems (non-C89) where stderr / stdout do not exist
  if (NULL == stderr) {
    // i give up printing
    if (NULL == stdout) {
      return;
    }

    fdredir(stderr, "/dev/stdout");
  }

  fprintf(stderr, "|\033[31m%s\033[0m|\033[31;1m \b%s:%" PRIu64 ": %s: %s: %s\033[0m\n",
    fatal ? "FATAL" : "ERROR", file, line, func, buf, errmsgs[errt]);

  safefree(buf);

  if ( fatal ) {
    fprintf(
      stderr,
      "\033[31m\nThat error was fatal, aborting.\n\n"
      "I'm melting!\033[0m\n"
    );
    abort();
  }

}

// uname, pass, realname
bankacct_t* bankacct_ctor (const char* const * const strdata, const uint16_t pin) {

  // neither arg can be 0
  if ( ! pin || NULL == strdata || NULL == strdata[0] || NULL == strdata[1] || NULL == strdata[2] ) {
    bank_error(strdata ? ERR_BAD_PIN : ERR_NO_STR, true, "%s", "");
    return NULL;
  }

  bankacct_t* bankacct = safemalloc( sizeof(bankacct_t) ); // 1

  const wallet_t       templ = { .creds = NULL, .debts = NULL, .is_frozen = false, .name = NULL, .flags = NULL };
  char**              nfo[3] = { &bankacct->uname, &bankacct->pass, &bankacct->realname };
  const char* const tnames[] = { "physical", "savings", "checking" };

  bankacct->num_wallets = INITAL_WALLETS;
  bankacct->wallets     = safemalloc( sizeof (wallet_t *) * INITAL_WALLETS ); // 2

  for (size_t i = 0; i < INITAL_WALLETS; i++) {
    wallet_t** twal = & (bankacct->wallets[i]);

    *twal = safemalloc( sizeof (wallet_t) ); // 3
    memcpy(*twal, &templ, sizeof (wallet_t));

    (*twal)->flags = make_empty_str();
    (*twal)->name  = strndup(tnames[i], 15); // 4
    (*twal)->num   = i;
    (*twal)->pin   = (uint16_t) i;

  }

  bankacct->fyz = bankacct->wallets[0],
  bankacct->sav = bankacct->wallets[1],
  bankacct->chk = bankacct->wallets[2];

  bankacct->pin               = pin;
  bankacct->max_walletid      = 2;
  bankacct->has_extra_wallets = false;
  bankacct->freeze_all        = false;
  bankacct->acct_u3id         = 1;

  bankacct->info = safemalloc( sizeof (nfo) ); // 5
  memcpy(bankacct->info, nfo, sizeof (nfo));

  for (size_t i = 0; i < 3; i++) {
    size_t thislen = safestrnlen(strdata[i]);
    *(bankacct->info[i]) = strndup(strdata[i], thislen); // 6
    //printf("%s\n", *(bankacct->info[i]));
  }

  report_ctor(bankacct);

  return bankacct;
}

void        bankacct_dtor (bankacct_t* const bankacct) {
  report_dtor(bankacct);

  wallet_dtorn(bankacct->wallets, bankacct->num_wallets);

  safefree_args(5, bankacct->info, bankacct->pass, bankacct->uname, bankacct->realname, bankacct);
}

char* bankacct_see (const bankacct_t* const bact) {

  // NOTE: exit point
  if ( ! bact ) {
    bank_error(ERR_NULL_STRUCT, true, "", "");
    return NULL;
  }

  char* out = safemalloc( sizeof(char) * 2000 );
  snprintf(
    out,
    1999,
    "bankacct #%zu \n\
    \tuname\t: %s \n\
    \tpass\t: %s \n\
    \tname\t: %s \n\
    \tpin\t: %d \n\
    \tuuuid\t: %zu \n\
    \tmax id\t: %" PRIu64 "\n\
    \textra\t: %s \n\
    \tfrozen\t: %s\n\
    ",
    bact->uid, bact->uname, bact->pass, bact->realname, bact->pin,
    bact->acct_u3id, bact->max_walletid,
    bact->has_extra_wallets ? "true" : "false",
    bact->freeze_all ? "true" : "false"
  );

  out = saferealloc(out, safestrnlen(out) + 1);

  return out;
}

void wallet_dtor (wallet_t* const wallet) {
  safefree_args(3, wallet->name, wallet->flags, wallet);
}

void wallet_dtorn (wallet_t* * const ws, size_t len) {
  for (size_t i = 0; i < len; i++) {
    wallet_dtor(ws[i]);
  }
  safefree(ws);
}
