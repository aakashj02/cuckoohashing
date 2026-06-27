#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>
#include <iostream>

template <typename Key, typename Value>
class CuckooHashTable {
private:
    struct Entry {
        Key key;
        Value value;
        bool is_occupied = false;
    };

    std::vector<Entry> table1;
    std::vector<Entry> table2;
    size_t capacity;
    size_t current_size;
    
    // Hash seeds changed during rehashing to break cyclic collisions
    size_t seed1 = 0x27d4eb2d;
    size_t seed2 = 0x9e3779b1;

    // O(log n) bounding is ideal, but a constant is standard for practical implementations
    const size_t MAX_KICKS = 500;

    size_t hash1(const Key& key) const {
        size_t h = std::hash<Key>{}(key);
        h ^= seed1;
        return h % capacity;
    }

    size_t hash2(const Key& key) const {
        size_t h = std::hash<Key>{}(key);
        // Bit-mixing ensures hash2 is sufficiently independent from hash1
        h ^= seed2;
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;
        return h % capacity;
    }

    void rehash() {
        size_t old_capacity = capacity;
        std::vector<Entry> old_table1 = std::move(table1);
        std::vector<Entry> old_table2 = std::move(table2);

        capacity = (old_capacity == 0) ? 8 : old_capacity * 2;
        
        // Mutate seeds to resolve the cycle that triggered the rehash
        seed1 ^= 0x12345678;
        seed2 ^= 0x87654321;

        table1.assign(capacity, Entry{});
        table2.assign(capacity, Entry{});
        current_size = 0;

        for (const auto& entry : old_table1) {
            if (entry.is_occupied) insert(entry.key, entry.value);
        }
        for (const auto& entry : old_table2) {
            if (entry.is_occupied) insert(entry.key, entry.value);
        }
    }

public:
    explicit CuckooHashTable(size_t initial_capacity = 8) 
        : capacity(initial_capacity), current_size(0) {
        table1.resize(capacity);
        table2.resize(capacity);
    }

    void insert(Key key, Value value) {
        // Prevent duplicate keys
        if (update_if_exists(key, value)) return;

        // Force resize if load factor is dangerously high (> 50% for 2-way Cuckoo is risky)
        if (current_size >= capacity) {
            rehash();
        }

        Entry current_entry{key, value, true};
        
        for (size_t i = 0; i < MAX_KICKS; ++i) {
            size_t pos1 = hash1(current_entry.key);
            if (!table1[pos1].is_occupied) {
                table1[pos1] = current_entry;
                ++current_size;
                return;
            }
            // Evict and take spot in table 1
            std::swap(current_entry, table1[pos1]);

            size_t pos2 = hash2(current_entry.key);
            if (!table2[pos2].is_occupied) {
                table2[pos2] = current_entry;
                ++current_size;
                return;
            }
            // Evict and take spot in table 2
            std::swap(current_entry, table2[pos2]);
            
            // Loop continues, attempting to place the newly evicted entry into table 1
        }

        // Cycle detected: rehash to get new hash functions/capacity, then insert the orphaned entry
        rehash();
        insert(current_entry.key, current_entry.value);
    }

    bool update_if_exists(const Key& key, const Value& value) {
        if (capacity == 0) return false;
        
        size_t p1 = hash1(key);
        if (table1[p1].is_occupied && table1[p1].key == key) {
            table1[p1].value = value;
            return true;
        }
        
        size_t p2 = hash2(key);
        if (table2[p2].is_occupied && table2[p2].key == key) {
            table2[p2].value = value;
            return true;
        }
        return false;
    }

    bool contains(const Key& key) const {
        if (capacity == 0) return false;
        
        size_t p1 = hash1(key);
        if (table1[p1].is_occupied && table1[p1].key == key) return true;
        
        size_t p2 = hash2(key);
        if (table2[p2].is_occupied && table2[p2].key == key) return true;
        
        return false;
    }

    bool remove(const Key& key) {
        if (capacity == 0) return false;

        size_t p1 = hash1(key);
        if (table1[p1].is_occupied && table1[p1].key == key) {
            table1[p1].is_occupied = false;
            --current_size;
            return true;
        }

        size_t p2 = hash2(key);
        if (table2[p2].is_occupied && table2[p2].key == key) {
            table2[p2].is_occupied = false;
            --current_size;
            return true;
        }

        return false;
    }

    size_t size() const { return current_size; }
};

// --- Example Usage ---
int main() {
    CuckooHashTable<int, std::string> map;
    
    map.insert(1, "One");
    map.insert(2, "Two");
    map.insert(3, "Three");

    std::cout << "Contains 2? " << (map.contains(2) ? "Yes" : "No") << '\n';
    
    map.remove(2);
    std::cout << "Contains 2 after removal? " << (map.contains(2) ? "Yes" : "No") << '\n';
    
    return 0;
}