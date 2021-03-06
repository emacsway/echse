#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdio.h>
#include "bitint.h"


int
main(int argc, char *argv[])
{
	bitint63_t x = {0U};

	x = ass_bi63(x, 36);
	printf("%llx %llx\n", x.pos, x.neg);

	x = ass_bi63(x, 40);
	printf("%llx %llx\n", x.pos, x.neg);

	x = ass_bi63(x, -4);
	printf("%llx %llx\n", x.pos, x.neg);

	x = ass_bi63(x, 0);
	printf("%llx %llx\n", x.pos, x.neg);
	x = ass_bi63(x, 1);
	printf("%llx %llx\n", x.pos, x.neg);
	x = ass_bi63(x, -33);
	printf("%llx %llx\n", x.pos, x.neg);
	return 0;
}

/* bitint_test.c ends here */
