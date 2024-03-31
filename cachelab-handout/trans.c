/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp[8];
    if(M == 32) {
        for (i = 0; i < 32; i += 8) {
            for (j = 0; j < 32; j += 8) {
                int l = i, r = i + 7;
                int L = j, R = j + 7;
                if(i != j) {
                    for(int I = l; I <= r; I ++) {
                        for(int J = L; J <= R; J ++) {
                            B[I][J] = A[J][I];
                        }
                    }
                } else {
                    for(int I = l; I <= r; I ++) {
                        for(int J = L; J <= R; J ++) {
                            tmp[J - L] = A[I][J];
                        }
                        for(int J = L; J <= R; J ++) {
                            B[I][J] = tmp[J - L];
                        }
                    }
                    for(int I = l; I <= r; I ++) {
                        for(int J = L; J <= R; J ++) {
                            tmp[0] = B[I][J];
                            B[I][J] = B[J][I];
                            B[J][I] = tmp[0];
                        }
                        L ++;
                    }
                }
            }
        }     
    } else if(M == 64) {
         for (i = 0; i < 64; i += 8) {
            for (j = 0; j < 64; j += 8) {
                for(int k = 0; k < 8; k ++) {
                    if(k < 4) {
                        for(int u = 0; u < 8; u ++)
                            tmp[u] = A[i + k][j + u];               
                        for(int u = 0; u < 4; u ++)
                            B[j + u][i + k] = tmp[u],
                            B[j + u][i + k + 4] = tmp[u + 4];
                    } else {
                         for(int u = 0; u < 4; u ++)
                            tmp[u + 4] = A[i + u + 4][j + k - 4];
                        for(int u = 0; u < 4; u ++) {
                            tmp[u] = B[j + k - 4][i + 4 + u];
                            B[j + k - 4][i + 4 + u] = tmp[u + 4];
                        }
                        for(int u = 4; u < 8; u ++)
                            tmp[u] = A[i + u][j + k];
                        for(int u = 0; u < 4; u ++)
                            B[j + k][i + u] = tmp[u],
                            B[j + k][i + u + 4] = tmp[u + 4];
                    }
                } 
            }
        }     
    } else if(M == 61) {
        for (i = 0; i < 67; i += 16) {
            for (j = 0; j < 61; j += 8) {
                int l = i, r = i + 15;
                int L = j, R = j + 7;
                r = r > 66 ? 66 : r;
                R = R > 60 ? 60 : R;
                for(int I = l; I <= r; I ++) {
                    for(int J = L; J <= R; J ++) {
                        tmp[J -L] = A[I][J];
                    }
                    for(int J = L; J <= R; J ++) {
                        B[J][I] = tmp[J - L];
                    }
                    
                }
            }
        }       
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

