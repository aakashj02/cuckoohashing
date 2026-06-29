# Cuckoo Hash Table (C++)

## What is Cuckoo Hashing?
Imagine a cuckoo bird: instead of building its own nest, it kicks another bird's eggs out of an existing nest to claim it. 

This data structure uses the exact same logic for memory management. Every piece of data (a key-value pair) is given exactly **two** possible "nests" (indices in two separate arrays, calculated by two different hash functions). 
1. If the first nest is empty, the data sits there.
2. If the nest is occupied, it "kicks" the current resident out.
3. The evicted resident then flies to its own alternate nest. If *that* is occupied, it kicks *that* resident out.
4. This chain reaction continues until every piece of data finds an empty spot.

## Why did we build this?
Standard hash tables (like C++ `std::unordered_map`) handle collisions by chaining items into linked lists. If too many items hash to the same spot, searching for an item degrades from an ideal O(1) to a worst-case O(n) time complexity. 

we implemented Cuckoo Hashing because it completely eliminates this problem. Because every item can only ever be in one of two specific locations, **lookups and deletions are guaranteed to take worst-case O(1) time.**

## Algorithmic Architecture
* **Collision Resolution:** Implements an eviction-based ("kicking") collision resolution strategy using two distinct hash functions with bit-mixing techniques.
* **Cycle Detection:** Utilizes a bounded eviction limit (`MAX_KICKS`) to dynamically detect cyclic collisions (infinite loops).
* **Dynamic Rehashing:** Automatically mutates hash seeds and scales table capacity when load factors trigger infinite loops, ensuring mathematical independence between table states.

## Time Complexity
| Operation | Average Case | Worst Case | 
| :--- | :--- | :--- |
| **Search / Contains** | O(1) | **O(1)** |
| **Deletion** | O(1) | **O(1)** |
| **Insertion** | O(1) | Amortized O(1) |

## Implementation Details
- Built with zero external dependencies (pure `<vector>`, `<functional>`, `<utility>`).
- Template-based architecture supporting arbitrary Key-Value data types.
- Memory efficient: Maintains an aggressive load factor threshold before triggering expansion overhead.

## How to Run
we have included a separate `test.cpp` file demonstrating the O(1) insertions, lookups, and the cycle-breaking rehashing logic. You can compile and run it using standard GCC:
`g++ test.cpp -o test_cuckoo`
`./test_cuckoo`

## References
* **Original Research Paper:** [Cuckoo Hashing (2001)](https://www.brics.dk/RS/01/32/BRICS-RS-01-32.pdf) by Rasmus Pagh and Flemming Friche Rodler.
