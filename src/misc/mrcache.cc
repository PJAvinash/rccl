#include "mrcache.hpp"

uintptr_t MemRegion::end() const {
    return start + length;
}

bool MemRegion::operator<(const MemRegion& other) const {
    return start < other.start;
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

void MemRegTable::removeByRegistration(void* registration){
    std::lock_guard<std::mutex> lock(mtx_);
    for(auto it = regions_.begin();it != regions_.end();++it){
        if(it->registration == registration){
            regions_.erase(it);
            break; // assuming each region will have unique registrations
        }
    }
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