typedef int tcflag_t;
typedef char cc_t;

#define NCCS 1

struct termios {
	tcflag_t c_iflag;      /* input modes */
	tcflag_t c_oflag;      /* output modes */
	tcflag_t c_cflag;      /* control modes */
	tcflag_t c_lflag;      /* local modes */
	cc_t     c_cc[NCCS];   /* special characters */
};

