#include "LRU.cpp"

template<typename Key, typename Value>
class LRUKcache : public LRUcache<Key, Value>{
private:
    int                                 k_;
    unique_ptr<LRUcache<Key, size_t>>   historyList_;
    unordered_map<Key, Value>           historyValueMap_;
public:
    LRUKcache(int capacity, int historyCapacity, int k)
    : LRUcache<Key, Value>(capacity)
    , historyList_(make_unique<LRUcache<Key, size_t>>(historyCapacity))
    , k_(k)
    {}
    
    
};