#ifndef MRCACHE_HPP
#define MRCACHE_HPP
#include <set>
#include <vector>
#include <cstdint>
#include <mutex>

struct MemRegion {
    uintptr_t start;
    size_t length;
    void* registration;
private:
    mutable int ref_count = 1;
public:
    MemRegion(uintptr_t start, size_t length, void* registration);
    uintptr_t end() const;
    bool operator<(const MemRegion& other) const;
    /*Increments reference counter for this Memory registration*/
    void retain() const;
    /*Decrements reference counter for this Memory registration and returns TRUE if zero references*/
    bool release() const;
    /*Returns number of live references*/
    int getRefCount() const;
};

class MemRegTable {
public:
    //Insert a region
    void insert(void* addr, size_t length, void* registration);
    //remove a region by starting address
    void remove(void* addr);
    bool removeByRegistration(void* registration);
    //Find all overlapping regions
    std::vector<const MemRegion*> find(void* addr, size_t length) const;
    //std::vector<MemRegion> findCopy(void* addr, size_t length) const;
    void lock();
    void unlock();
private:
    mutable std::mutex mtx_;
    std::set<MemRegion> regions_;
};

#endif //MRCACHE_HPP