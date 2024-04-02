/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "holic0105",              /* Team name */

    "Holic",     /* First member full name */
    "3485995896@qq.com",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    int dim2 = dim << 1;
    for(int i = 1; i <= dim; i += 4) {
        int x2 = i;
        int x3 = x2 + 1;
        int x1 = x2 - 1;
        int x4 = x3 + 1;

        int y1 = i * dim;
        int y2 = y1 + dim;
        int y3 = y2 + dim;
        int y4 = y3 + dim;

        for(int j = 0; j < dim; j += 2) {
            dst[x1] = src[y1 - 1];
            dst[x2] = src[y2 - 1]; 
            dst[x3] = src[y3 - 1];
            dst[x4] = src[y4 - 1];
            dst[x1 + dim] = src[y1 - 2];
            dst[x2 + dim] = src[y2 - 2];
            dst[x3 + dim] = src[y3 - 2];
            dst[x4 + dim] = src[y4 - 2];
            x1 += dim2; y1 -= 2;
            x2 += dim2; y2 -= 2;
            x3 += dim2; y3 -= 2;
            x4 += dim2; y4 -= 2;
        }
    }
}


/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);     
    /* ... Register additional test functions here */
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++) 
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++) 
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);
}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */
static void  corner(pixel *dst, pixel *src, int t, int s, int dim) {
    int t1 = t + 1; // 右上
    int t2 = t + dim; //左下
    int t3 = t2 + 1; //右下
    dst[s].red  = ((src[t].red + src[t1].red) + (src[t2].red + src[t3].red)) >> 2;
    dst[s].green =((src[t].green + src[t1].green) + (src[t2].green + src[t3].green)) >> 2;
    dst[s].blue = ((src[t].blue + src[t1].blue) + (src[t2].blue + src[t3].blue)) >> 2;
}
static void edgeL(pixel *dst, pixel *src, int t, int s, int dim) {
    int t1 = t + 1;
    int t2 = t + dim;
    int t3 = t2 + 1;
    int t4 = t2 + dim;
    int t5 = t4 + 1;
    dst[s].red  = ((src[t].red + src[t1].red) + (src[t2].red + src[t3].red) + (src[t4].red + src[t5].red)) / 6;
    dst[s].green = ((src[t].green + src[t1].green) + (src[t2].green + src[t3].green) + (src[t4].green + src[t5].green)) / 6;
    dst[s].blue  = ((src[t].blue + src[t1].blue) + (src[t2].blue + src[t3].blue) + (src[t4].blue + src[t5].blue)) /6 ;
}
static void  edgeH(pixel *dst, pixel *src, int t, int s, int dim) {
    int t1 = t + 1;
    int t2 = t + dim;
    int t3 = t2 + 1;
    int t4 = t1 + 1;
    int t5 = t3 + 1;
    dst[s].red  = ((src[t].red + src[t1].red) + (src[t2].red + src[t3].red) + (src[t4].red + src[t5].red)) / 6;
    dst[s].green = ((src[t].green + src[t1].green) + (src[t2].green + src[t3].green) + (src[t4].green + src[t5].green)) / 6;
    dst[s].blue  = ((src[t].blue + src[t1].blue) + (src[t2].blue + src[t3].blue) + (src[t4].blue + src[t5].blue)) /6 ;
}
static void  solve(pixel *dst, pixel *src, int t, int s, int dim) {
    int t1 = t + 1; //中上
    int t2 = t + dim; //左中
    int t3 = t2 + 1; //中中
    int t4 = t1 + 1; //右上
    int t5 = t3 + 1; //右中
    int t6 = t2 + dim; //左下
    int t7 = t6 + 1; 
    int t8 = t7 + 1;
    dst[s].red = (((src[t].red + src[t1].red) + (src[t2].red + src[t3].red))
      + ((src[t4].red + src[t5].red) + (src[t6].red + src[t7].red))
      + src[t8].red) / 9;
    dst[s].green = (((src[t].green + src[t1].green) + (src[t2].green + src[t3].green))
      + ((src[t4].green + src[t5].green) + (src[t6].green + src[t7].green))
      + src[t8].green) / 9;
    dst[s].blue = (((src[t].blue + src[t1].blue) + (src[t2].blue + src[t3].blue))
      + ((src[t4].blue + src[t5].blue) + (src[t6].blue + src[t7].blue))
      + src[t8].blue) / 9;
}
char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) 
{
    int s = 0, t = 0;
    corner(dst , src, s, t, dim);
    
    t = dim - 1, s = t - 1;
    corner(dst , src, s, t, dim);
    
    t = dim * dim - 1, s = t - dim - 1;
    corner(dst , src, s, t, dim);

    t -= dim - 1, s = t - dim;
    corner(dst , src, s, t, dim);

    int maxn = dim * dim;
    for(t = dim * (dim - 1) - 1; s > 0; s -= dim, t -= dim) {
        edgeL(dst, src, s - dim, s, dim);
        edgeL(dst, src, t - dim - 1, t, dim);
    }

    t = maxn - 2;
    s = dim - 2;
    while(s > 0) {
        edgeH(dst, src, s - 1, s, dim);
        edgeH(dst, src, t - 1 - dim, t, dim);
        s --;
        t --;
    }
    s = dim + 1, t = 0; 
    for (int i = 1; i < dim - 1; i ++) {
	    for (int j = 1; j < dim - 1; j++) {
			solve(dst, src, t, s, dim);
            s ++;
            t ++;
		}
        s += 2;
        t += 2;
	}
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
}

