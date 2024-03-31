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

typedef struct{
    bool valid_;
    long long tag_;
    int lru_value_;
} Line;

typedef struct {
    Line *line_;
    int num_;
} Set;

typedef struct {
    Set *set;
    int time_;
    int s_;
    int E_;
    int b_;
    int S_;
    int sb_;
    int hitcount_;
    int misscount_;
    int evictioncout_;
} Cache;

int GetSetIndex(Cache *cache, const long long *addr) {
    return (*addr >> cache->b_) & (cache->S_);
}

long long GetTagIndex(Cache *cache, const long long *addr) {
    return *addr >> cache->sb_;
}

int LRU(Cache *cache, const int set_index) {
    int id = 0;
    for(int i = 1; i < cache->E_; i ++) {
        if(cache->set[set_index].line_[i].lru_value_ < cache->set[set_index].line_[id].lru_value_)
            id = i;
    }
    return id;
}

bool IsInCache(Cache *cache, const int set_index, const long long  *tag_index) {
    for(int i = 0; i < cache->E_; i ++) {
        if(cache->set[set_index].line_[i].valid_ == false) continue;
        if(cache->set[set_index].line_[i].tag_ == *tag_index) {
            cache->set[set_index].line_[i].lru_value_ = cache->time_;
            return true;
        }
    }
    return false;
}

void InsertCache(Cache *cache, const int set_index, const long long *tag_index) {
    if(flagv) printf("miss ");
    cache->misscount_ ++;
    if(cache->set[set_index].num_ < cache->E_) {
        cache->set[set_index].line_[cache->set[set_index].num_ ++] = (Line){true, *tag_index, cache->time_};
    } else {
        int id = LRU(cache, set_index);
         cache->set[set_index].line_[id] = (Line){true, *tag_index, cache->time_};
        if(flagv) printf("evictions ");
        cache->evictioncout_++;
    }
}

void Load(Cache *cache, const int set_index, const long long *tag_index) {
    cache->time_ ++;
    if(IsInCache(cache, set_index, tag_index)) {
        cache->hitcount_ ++;
        if(flagv) printf("hit ");
    } else {
        InsertCache(cache, set_index, tag_index);
    }
}

void Store(Cache *cache, const int set_index, const long long *tag_index) {
    cache->time_ ++;
    if(IsInCache(cache, set_index, tag_index)) {
        cache->hitcount_ ++;
        if(flagv) printf("hit ");
    } else {
        InsertCache(cache, set_index, tag_index);
    }
}

void Modify(Cache *cache, const int set_index, const long long *tag_index) {
    Load(cache, set_index, tag_index);
    Store(cache, set_index, tag_index);
}

void InitCache(Cache *cache){
    cache->hitcount_ = 0;
    cache->misscount_ = 0;
    cache->evictioncout_ = 0;

    cache->S_ = (1 << cache->s_) - 1;
    cache->sb_ = cache->s_ + cache->b_;

    cache->set = (Set *) malloc(sizeof(Set) * (cache->S_ + 1));

    for(int i = 0; i <= cache->S_; i ++) {
        cache->set[i].line_ = (Line *) malloc(sizeof(Line) * cache->E_);
        cache->set[i].num_ = 0;
        for(int j = 0; j < cache->E_; j ++)
            cache->set[i].line_[j].valid_ = false;
    }
}

void ClearCache(Cache * cache) {

    for(int i = 0; i <= cache->S_; i ++) {
        free(cache->set[i].line_);
    }

    free(cache->set);

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
    char opt[5];

    InitCache(cache);

    while(~scanf("%s %llx,%d", opt, &addr, &siz)) {
        int set_index = GetSetIndex(cache, &addr);
        long long tag_index = GetTagIndex(cache, &addr);
        if(flagv) printf("%s %llx,%d ", opt, addr, siz);
        switch (opt[0])
        {
            case 'L':
                Load(cache, set_index, &tag_index);
                break;
            case 'S':
                Store(cache, set_index, &tag_index);
                break;
            case 'M':
                Modify(cache, set_index, &tag_index);
                break;
            default:
                break;
        }
        if(flagv) printf("\n");
    }

    ClearCache(cache);
}

int main(int argc, char *argv[])
{
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    if(GetOpt(argc, argv, cache) == false)
        return 0;
    freopen(filename, "r", stdin);
    Solve(cache);
    printSummary(cache->hitcount_, cache->misscount_, cache->evictioncout_);
    return 0;
}
