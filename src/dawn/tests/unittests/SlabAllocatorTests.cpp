// Copyright 2020 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <set>
#include <thread>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/common/SlabAllocator.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

struct Foo : public PlacementAllocated {
    explicit Foo(int value) : value(value) {}

    int value;
};

struct alignas(256) AlignedFoo : public Foo {
    using Foo::Foo;
};

// Test that a slab allocator of a single object works.
TEST(SlabAllocatorTests, Single) {
    SlabAllocator<Foo> allocator(1 * sizeof(Foo));

    Foo* obj = allocator.Allocate(4);
    EXPECT_EQ(obj->value, 4);

    allocator.Deallocate(obj);
}

// Allocate multiple objects and check their data is correct.
TEST(SlabAllocatorTests, AllocateSequential) {
    // Check small alignment
    {
        SlabAllocator<Foo> allocator(5 * sizeof(Foo));

        std::vector<Foo*> objects;
        for (int i = 0; i < 10; ++i) {
            auto* ptr = allocator.Allocate(i);
            EXPECT_TRUE(std::find(objects.begin(), objects.end(), ptr) == objects.end());
            objects.push_back(ptr);
        }

        for (int i = 0; i < 10; ++i) {
            // Check that the value is correct and hasn't been trampled.
            EXPECT_EQ(objects[i]->value, i);

            // Check that the alignment is correct.
            EXPECT_TRUE(IsPtrAligned(objects[i], alignof(Foo)));
        }

        // Deallocate all of the objects.
        for (Foo* object : objects) {
            allocator.Deallocate(object);
        }
    }

    // Check large alignment
    {
        SlabAllocator<AlignedFoo> allocator(9 * sizeof(AlignedFoo));

        std::vector<AlignedFoo*> objects;
        for (int i = 0; i < 21; ++i) {
            auto* ptr = allocator.Allocate(i);
            EXPECT_TRUE(std::find(objects.begin(), objects.end(), ptr) == objects.end());
            objects.push_back(ptr);
        }

        for (int i = 0; i < 21; ++i) {
            // Check that the value is correct and hasn't been trampled.
            EXPECT_EQ(objects[i]->value, i);

            // Check that the alignment is correct.
            EXPECT_TRUE(IsPtrAligned(objects[i], 256));
        }

        // Deallocate all of the objects.
        for (AlignedFoo* object : objects) {
            allocator.Deallocate(object);
        }
    }
}

// Test that when reallocating a number of objects <= pool size, all memory is reused.
TEST(SlabAllocatorTests, ReusesFreedMemory) {
    SlabAllocator<Foo> allocator(17 * sizeof(Foo));

    // Allocate a number of objects.
    std::set<Foo*> objects;
    for (int i = 0; i < 17; ++i) {
        EXPECT_TRUE(objects.insert(allocator.Allocate(i)).second);
    }

    // Deallocate all of the objects.
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }

    std::set<Foo*> reallocatedObjects;
    // Allocate objects again. All of the pointers should be the same.
    for (int i = 0; i < 17; ++i) {
        Foo* ptr = allocator.Allocate(i);
        EXPECT_TRUE(reallocatedObjects.insert(ptr).second);
        EXPECT_TRUE(std::find(objects.begin(), objects.end(), ptr) != objects.end());
    }

    // Deallocate all of the objects.
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }
}

// Test many allocations and deallocations. Meant to catch corner cases with partially
// empty slabs.
TEST(SlabAllocatorTests, AllocateDeallocateMany) {
    SlabAllocator<Foo> allocator(17 * sizeof(Foo));

    std::set<Foo*> objects;
    std::set<Foo*> set3;
    std::set<Foo*> set7;

    // Allocate many objects.
    for (uint32_t i = 0; i < 800; ++i) {
        Foo* object = allocator.Allocate(i);
        EXPECT_TRUE(objects.insert(object).second);

        if (i % 3 == 0) {
            set3.insert(object);
        } else if (i % 7 == 0) {
            set7.insert(object);
        }
    }

    // Deallocate every 3rd object.
    for (Foo* object : set3) {
        allocator.Deallocate(object);
        objects.erase(object);
    }

    // Allocate many more objects
    for (uint32_t i = 0; i < 800; ++i) {
        Foo* object = allocator.Allocate(i);
        EXPECT_TRUE(objects.insert(object).second);

        if (i % 7 == 0) {
            set7.insert(object);
        }
    }

    // Deallocate every 7th object from the first and second rounds of allocation.
    for (Foo* object : set7) {
        allocator.Deallocate(object);
        objects.erase(object);
    }

    // Allocate objects again
    for (uint32_t i = 0; i < 800; ++i) {
        Foo* object = allocator.Allocate(i);
        EXPECT_TRUE(objects.insert(object).second);
    }

    // Deallocate the rest of the objects
    for (Foo* object : objects) {
        allocator.Deallocate(object);
    }
}

// Test many allocations and deallocations in a multithreaded environment.
TEST(SlabAllocatorTests, AllocateDeallocateManyMultithread) {
    static constexpr uint32_t kNumObjectsPerThread = 100;
    static constexpr uint32_t kNumThreads = 10;

    SlabAllocator<Foo> allocator(17 * sizeof(Foo));
    auto f = [&] {
        std::set<Foo*> objects;
        std::set<Foo*> set2;

        // Allocate many objects.
        for (uint32_t i = 0; i < kNumObjectsPerThread; i++) {
            Foo* object = allocator.Allocate(i);
            EXPECT_TRUE(objects.insert(object).second);
            if (i % 2 == 0) {
                set2.insert(object);
            }
        }

        // Deallocate every other object.
        for (Foo* object : set2) {
            allocator.Deallocate(object);
            objects.erase(object);
        }

        // Allocate more objects.
        for (uint32_t i = 0; i < kNumObjectsPerThread / 2; i++) {
            Foo* object = allocator.Allocate(i);
            EXPECT_TRUE(objects.insert(object).second);
        }

        // Deallocate the rest of the objects
        for (Foo* object : objects) {
            allocator.Deallocate(object);
        }
    };

    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < kNumThreads; i++) {
        threads.emplace_back(f);
    }
    for (uint32_t i = 0; i < kNumThreads; i++) {
        threads[i].join();
    }
}

}  // anonymous namespace
}  // namespace dawn
