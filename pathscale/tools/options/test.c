/*
 * Copyright 2004, 2005, 2006 PathScale, Inc.  All Rights Reserved.
 * Unpublished -- rights reserved under the copyright laws of the
 * United States. USE OF A COPYRIGHT NOTICE DOES NOT IMPLY PUBLICATION
 * OR DISCLOSURE. THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND
 * TRADE SECRETS OF PATHSCALE, INC. USE, DISCLOSURE, OR REPRODUCTION
 * IS PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF
 * PATHSCALE, INC.
 *
 * U.S. Government Restricted Rights:
 * The Software is a "commercial item," as that term is defined at 48
 * C.F.R. 2.101 (OCT 1995), consisting of "commercial computer
 * software" and "commercial computer software documentation," as such
 * terms are used in 48 C.F.R. 12.212 (SEPT 1995).  Consistent with 48
 * C.F.R. 12.212 and 48 C.F.R. 227-7202-1 through 227-7202-4 (JUNE
 * 1995), all U.S. Government End Users acquire the Software with only
 * those rights set forth in the accompanying license agreement.
 *
 * PathScale, Inc. 2071 Stierlin Court, Suite 200; Mountain View CA 94043.
 */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

int max = 0;
int size = 0;
double *array = NULL;

void record(double a)
{
    if (max == size) {
        if (array == NULL) {
            array = malloc(1000*sizeof (double));
            max = 1000;
        } else {
            array = realloc(array, sizeof (double) * max * 2);
            max *= 2;
        }
    }
    array[size++] = a;
}

void print()
{
    FILE *fp = fopen("qqqq", "w");
    int i;
    for (i = 0; i < size; i++) {
        fprintf(fp, "%g\n", array[i]) ;
    }
    fclose(fp);
}

// simple iterative factorial calculator.  Just for timing
int main()
{
    int i;
    time_t start = time(NULL);
    for (i = 2; i < 50000; i++) {
        double f = 1.0;
        int n = i;
        while (n-- > 1) {
            f *= n;
        }
        record (f);
    }
    print();
    time_t end = time(NULL);
    printf("%g\n", (double)(end - start));
    return 0;
}