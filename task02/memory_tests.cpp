#include "tree.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>

namespace {

long long allocation_budget = -1;
std::size_t live_allocations = 0;

class AllocationFailure {
public:
    explicit AllocationFailure(long long budget) noexcept {
        allocation_budget = budget;
    }

    AllocationFailure(const AllocationFailure&) = delete;
    AllocationFailure& operator=(const AllocationFailure&) = delete;

    ~AllocationFailure() {
        allocation_budget = -1;
    }
};

}

void* operator new(std::size_t size) {
    if (allocation_budget == 0) {
        throw std::bad_alloc();
    }
    if (allocation_budget > 0) {
        --allocation_budget;
    }
    void* memory = std::malloc(size == 0 ? 1 : size);
    if (memory == nullptr) {
        throw std::bad_alloc();
    }
    ++live_allocations;
    return memory;
}

void operator delete(void* memory) noexcept {
    if (memory != nullptr) {
        --live_allocations;
        std::free(memory);
    }
}

void* operator new[](std::size_t size) {
    return ::operator new(size);
}

void operator delete[](void* memory) noexcept {
    ::operator delete(memory);
}

void operator delete(void* memory, std::size_t) noexcept {
    ::operator delete(memory);
}

void operator delete[](void* memory, std::size_t) noexcept {
    ::operator delete(memory);
}

template <class Action, class Verify>
void check_failures(Action action, Verify verify) {
    const std::size_t baseline = live_allocations;
    bool observed_failure = false;
    bool observed_success = false;
    for (long long budget = 0; budget < 64; ++budget) {
        try {
            const AllocationFailure failure(budget);
            action();
            observed_success = true;
        } catch (const std::bad_alloc&) {
            observed_failure = true;
        }
        assert(live_allocations == baseline);
        verify();
    }
    assert(observed_failure && observed_success);
}


int main() {
    const std::size_t baseline = live_allocations;
    {
        SearchTree tree;
        for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
            tree.insert(value);
        }
        auto verify = [&] {
            assert(tree.size() == 9 && tree.height() == 4);
            for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
                assert(tree.contains(value));
            }
            assert(!tree.contains(100));
        };
        check_failures([&] {
            SearchTree copy(tree);
            copy.insert(100);
        }, verify);
        check_failures([&] {
            SearchTree destination;
            destination.insert(-100);
            try {
                destination = tree;
            } catch (...) {
                assert(destination.size() == 1 && destination.contains(-100));
                throw;
            }
        }, verify);
        check_failures([&] {
            tree.insert(100);
            tree.erase(100);
        }, verify);
    }
    assert(live_allocations == baseline);
    std::puts("Task 02: allocation failures handled, no leaked allocations");
}
