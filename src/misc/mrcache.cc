#include "mrcache.hpp"

uintptr_t MemRegion::end() const {
    return start + length;
}

bool MemRegion::operator<(const MemRegion& other) const {
    return start < other.start;
}

MemRegion::MemRegion(uintptr_t s, size_t len, void* reg) : start(s), length(len), registration(reg) {}

void MemRegion::retain() const {
    __sync_fetch_and_add(&ref_count, 1);
}

bool MemRegion::release() const {
    return __sync_sub_and_fetch(&ref_count, 1) == 0;
}

int MemRegion::getRefCount() const {
    return __sync_fetch_and_add(const_cast<int*>(&ref_count), 0);
}

void MemRegTable::insert(void* addr, size_t length, void* registration) {
    uintptr_t start = reinterpret_cast<uintptr_t>(addr);
    std::lock_guard<std::mutex> lock(mtx_);
    regions_.insert(MemRegion{start, length, registration});
}

void MemRegTable::remove(void* addr) {
    uintptr_t start = reinterpret_cast<uintptr_t>(addr);
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = regions_.lower_bound(MemRegion{start, 0, nullptr});
    if (it != regions_.end() && it->start == start) {
        regions_.erase(it);
    }
}

bool MemRegTable::removeByRegistration(void* registration){
    std::lock_guard<std::mutex> lock(mtx_);
    bool removed = false;
    for(auto it = regions_.begin();it != regions_.end();++it){
        if(it->registration == registration){ // assuming each region will have unique registrations
            if(it->release()){
                regions_.erase(it);
                return true;
            }
            return false; 
        }
    }
    return false;
}

std::vector<const MemRegion*> MemRegTable::find(void* addr, size_t length) const {
    uintptr_t query_start = reinterpret_cast<uintptr_t>(addr);
    uintptr_t query_end = query_start + length;
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<const MemRegion*> result;

    auto it = regions_.lower_bound(MemRegion{query_start - length, 0, nullptr});
    for (; it != regions_.end(); ++it) {
        if (it->start >= query_end) break;
        if (it->end() > query_start && it->start < query_end) {
            result.push_back(&(*it));
        }
    }
    return result;
}