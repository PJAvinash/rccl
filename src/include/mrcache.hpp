#ifndef MRCACHE_HPP
#define MRCACHE_HPP
#include <set>
#include <vector>
#include <cstdint>
#include <mutex>

struct MemRegion{
    uintptr_t start;
    size_t length;
    void* registration;
    uintptr_t end() const;
    bool operator<(const MemRegion& other) const;
};

class MemRegTable {
public:
    //Insert a region
    void insert(void* addr, size_t length, void* registration);
    //remove a region by starting address
    void remove(void* addr);
    void removeByRegistration(void* registration);
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