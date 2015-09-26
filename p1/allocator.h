#pragma once

#include <stdexcept>
#include <string>
#include <list>
#include <stdlib.h> 
#include <map>

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

class Pointer {
private:
    static std::map <unsigned int, void*> id2p;
    unsigned int id = 0;
    size_t size = 0;
public:
    Pointer() {id = rand(); id2p[id] = nullptr;}
    Pointer(void *_ptr) {id = rand(); id2p[id] = _ptr;}
    Pointer(void *_ptr, size_t _size) {id = rand(); id2p[id] = _ptr; size = _size;}
    Pointer(void *_ptr, size_t _size, unsigned int _id) {id = _id; id2p[id] = _ptr; size = _size;}
    void *get() const { return id2p[id]; }
    size_t get_size() const { return size; }
    unsigned int get_id() { return id; }
    void set_size(size_t _size) { size = _size; }
    bool operator==(const Pointer &other) const {
        return get() == other.get();
    }
};

class Allocator {
private:
    void *base = nullptr;
    size_t size = 0;
    std::list <Pointer> plist;

public:
    Allocator(void *_base, size_t _size) {
        base =_base;
        size = _size;
        plist.push_back(Pointer(_base));
    }
    
    Pointer alloc(size_t N) {
        auto i1 = plist.begin();
        auto i2 = plist.begin();
        i2++;

        while (i2 != plist.end()) {
            if (reinterpret_cast<char*>((*i2).get()) - reinterpret_cast<char*>((*i1).get()) - (*i1).get_size() >= N) {
                Pointer new_p = Pointer(reinterpret_cast<char*>((*i1).get()) + (*i1).get_size(), N);
                plist.insert(i2, new_p);
                return new_p;
            }
            i1++;
            i2++;
        }

        Pointer p = plist.back();
        if (reinterpret_cast<char*>(p.get()) + N > reinterpret_cast<char*>(base) + size) {
            throw AllocError(AllocErrorType::NoMemory, "No memory for allocation");
        }
        p.set_size(N);
        plist.pop_back();
        plist.push_back(p);
        plist.push_back(Pointer(reinterpret_cast<char*>(p.get()) + N, 0));
        return p;
    }
    void realloc(Pointer &p, size_t N) {
        for (auto it = plist.begin(); it != plist.end(); it++) {
            if ((*it) == p) {
                plist.erase(it);
                break;
            }
        }
        p = alloc(N);
    }
    void free(Pointer &p) {
        for (auto it = plist.begin(); it != plist.end(); it++) {
            if ((*it) == p) {
                plist.erase(it);
                p = Pointer();
                break;
            }
        }
    }

    void defrag() {
        Pointer last = Pointer(base, 0);
        for (auto it = plist.begin(); it != plist.end(); it++) {
            if (reinterpret_cast<char*>((*it).get()) - reinterpret_cast<char*>(last.get()) - last.get_size()) {
                memmove(reinterpret_cast<char*>(last.get()) + last.get_size(), (*it).get(), (*it).get_size());
                *it = Pointer(reinterpret_cast<char*>(last.get()) + last.get_size(), (*it).get_size(), (*it).get_id());
                last = (*it);
            }
        }

    }
    std::string dump() { return ""; }
};

