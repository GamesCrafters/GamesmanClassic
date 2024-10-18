#include <stdio.h>
#include <stdlib.h>

main(int argc, char ** argv)
{
	FILE * filep;
	int n, i=0, a;

	char mystring [255];

	if(argc == 1)
		printf("Usage: %s <module name>\n", argv[0]), exit(1);

	printf("\n\nGAMESMAN automatic game solver, written by Sunil Ramesh\n\n");

	sprintf(mystring, "./%s -numberOfOptions > .solvetemp", argv[1]);

	if(system(mystring) == 0) printf("Invalid module. Quitting\n"), exit(1);

	if((filep = fopen(".solvetemp", "r")) == NULL)
		printf("Sorry, an internal error occurred.\n"), exit(1);

	fscanf(filep, "%d", &n);
	fclose(filep);
	remove(".solvetemp");

	printf("Solving %s, %d options:\n\n", argv[1], n);

	a = printf("Finished %d of %d Games (%3.0f%%)", i, n, 100*(float)i/(float)n);

	fflush(stdout);

	for(i = 1; i <= n; i++)
	{
		for(; a >= 0; a--) printf("\b");
		fflush(stdout);
		sprintf(mystring, "./%s -s %d", argv[1], i);
		system(mystring);
		a = printf("Finished %d of %d Games (%3.0f%%)", i, n, 100*(float)i/(float)n);
		fflush(stdout);
	}
	printf("\n\nDone solving %s\n\nThanks for using GAMESMAN automatic game solver !\n\n", argv[1]);
}
