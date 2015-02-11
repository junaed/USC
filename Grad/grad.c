/*
 * File:   main.c
 * Author: junaed
 *
 * Created on March 2, 2014, 10:03 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 *
 */

double Log2(double n) {
	// log(n)/log(2) is log2.
	return log(n) / log(2);
}

int main(int argc, char** argv) {

	int p, n, serial;
	double parallel, speedup;
	printf("p\t\tn\t\tT(serial)\tT(parallel)\t\tSpeedUp\t\tEfficiency\n");
	printf(
			"=================================================================================================\n");
	for (p = 1; p <= 128; p *= 2) {
		for (n = 10; n <= 320; n *= 2) {
			serial = n * n;
			printf("%3d", p);
			printf("\t\t%3d", n);
			printf("\t\t%6d", serial);
			parallel = (((double) n * (double) n) / p) + Log2((double) p);
			printf("\t\t%9.2f", parallel);
			speedup = serial / parallel;
			printf("\t\t%8.3f", speedup);
			printf("\t%8.3f\n", speedup / p);
		}
	}

	printf(
			"========================================================================================\n\n\n\n");

	printf("\n\nSPEEDUP\n\n");

	printf("p\t%8d\t%8d\t%8d\t%8d\t%8d\t%8d\n", 10, 20, 40, 80, 160, 320);
	printf(
			"=====================================================================================================\n");

	for (p = 1; p <= 128; p *= 2) {
		printf("%3d", p);
		for (n = 10; n <= 320; n *= 2) {
			serial = n * n;

			parallel = (((double) n * (double) n) / p) + Log2((double) p);
			printf("\t%8.2f", serial / parallel);
		}
		printf("\n");
	}

	printf("\n\nEFFICIENCY\n\n");

	printf("p\t%8d\t%8d\t%8d\t%8d\t%8d\t%8d\n", 10, 20, 40, 80, 160, 320);
	printf(
			"=====================================================================================================\n");

	for (p = 1; p <= 128; p *= 2) {
		printf("%3d", p);
		for (n = 10; n <= 320; n *= 2) {
			serial = n * n;

			parallel = (((double) n * (double) n) / p) + Log2((double) p);
			printf("\t%8.2f", (serial / (p * parallel)));
		}
		printf("\n");
	}

	return (EXIT_SUCCESS);
}
