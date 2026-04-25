#pragma once

struct Location {
    long long pageNumber;
    int offset;
    
    Location() : pageNumber(-1), offset(-1) {}
    Location(long long page, int off) : pageNumber(page), offset(off) {}
};