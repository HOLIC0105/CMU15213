#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define SIZE_FILENAME  105

bool flagv;

char filename[SIZE_FILENAME];

typedef struct {
    int s_;
    int E_;
    int b_;
    int S_;
    int sb_;
    int hitcount_;
    int misscount_;
    int evictioncout_;
} Cache;

inline int GetSetIndex(Cache *cache, const long long *addr) {
    return *addr >> cache->b_ & cache->S_;
}

inline long long GetTagIndex(Cache *cache, const long long *addr) {
    return *addr >> (cache->b_ + cache->sb_);
}

void InitCache(Cache *cache){
    cache->hitcount_ = 0;
    cache->misscount_ = 0;
    cache->evictioncout_ = 0;

    cache->S_ = (1 << cache->s_) - 1;
    cache->sb_ = cache->s_ + cache->b_;
}

void HelpPrint() {
    puts("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>");
    puts("Options:");
    puts("-h         Print this help message.");
    puts("-v         Optional verbose flag.");
    puts("-s <num>   Number of set index bits.");
    puts("-E <num>   Number of lines per set.");
    puts("-b <num>   Number of block offset bits.");
    puts("-t <file>  Trace file.");
}

bool GetOpt(int argc, char *argv[], Cache *cache) {
    const char * optstring = "hvs:E:b:t:";
    int o;
    while((o = getopt(argc, argv, optstring)) != -1) {
        switch (o)
        {
            case 'h':
                HelpPrint();
                break;
            case 'v':
                flagv = 1;
                break;
            case 's':
                cache->s_ = atoi(optarg);
                break;
            case 'E':
                cache->E_ = atoi(optarg);
                break;
            case 'b':
                cache->b_ = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
            case '?':
                printf("invalid option -- '%s'", optarg);
                return false;
                break;
        }
    }
    return true;
}

void Solve(Cache *cache) {
    long long addr;
    int siz;
    char opt;

    InitCache(cache);

    while(~scanf("%c %llx,%d", &opt, &addr, &siz)) {
        int set_index = GetSetIndex(cache, &addr);
        int tag_index = GetTagIndex(cache, &addr);
        if(flagv) printf("%c %llx,%d", opt, addr, siz);
        switch (opt)
        {
        case 'L':
            /* code */
            break;
        
        default:
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    if(GetOpt(argc, argv, cache) == false)
        return 0;
    freopen(filename, "r", stdin);
    Solve(cache);
    printSummary(0, 0, 0);
    return 0;
}
