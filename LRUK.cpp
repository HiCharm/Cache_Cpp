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

    Value get(Key key){
        Value value{};
        bool inMainCache = LRUcache<Key,Value>::get(key, value);

        size_t historyCount = historyList_->get(key);
        historyCount++;
        historyList_->put(key, historyCount);

        if(inMainCache){
            return value;
        }

        if(historyCount >= k_){
            auto it = historyValueMap_.find(key);
            if(it != historyValueMap_.end()){
                Value storedValue = it->second;
                historyList_->remove(key);
                historyValueMap_.erase(it);
                LRUcache<Key,Value>::put(key,storedValue);
                return storedValue;
            }
        }
        return value;
    }

    void put(Key key, Value value){
        Value existingValue{};
        bool inMainCache = LRUcache<Key,Value>::get(key,existingValue);
        if(inMainCache){
            LRUcache<Key,Value>::put(key,value);
            return;
        }

        size_t historyCount = historyList_->get(key);
        historyCount++;
        historyList_->put(key,historyCount);

        historyValueMap_[key] = value;

        if(historyCount >= k_){
            historyList_->remove(key);
            historyValueMap_.erase(key);
            LRUcache<Key,Value>::put(key,value);
        }
    }
};