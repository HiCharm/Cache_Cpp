
template<typename Key, typename Value> class LRUcache;

#include "CachePolicy.cpp"

#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

using namespace std;
template<typename Key, typename Value>
class LRUnode{
private:
    Key key_;
    Value value_;
    size_t accessCount_;
    weak_ptr<LRUnode<Key, Value>> prev_;
    shared_ptr<LRUnode<Key, Value>> next_;
public:
    LRUnode(Key key, Value value)
    : key_(key)
    , value_(value)
    , accessCount_(1)
    {}

    Key getKey() const { return key_;}
    Value getValue() const { return value_;}
    void setValue(const Value &value) { value_ = value;}
    size_t getAccessCount() const { return accessCount_;}
    void incrementAccessCount() { ++accessCount_;}

    friend class LRUcache<Key, Value>;
};

template<typename Key, typename Value>
class LRUcache : public CachePolicy<Key, Value>{
public:
    using LruNodeType = LRUnode<Key, Value>;
    using NodePtr = shared_ptr<LruNodeType>;
    using NodeMap = unordered_map<Key,NodePtr>;

    LRUcache(int capacity):capacity_(capacity){
        initializeList();
    }
    ~LRUcache() override = default;
    
    void put(Key key, Value value) override{
        if(capacity_ <= 0) return;
        lock_guard<mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if(it != nodeMap_.end()){
            // in the cache
            updateExistingNode(it->second,value);
            return;
        }
        addNewNode(key,value);
    }
    bool get(Key key, Value &value) override{
        lock_guard<mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if(it != nodeMap_.end()){
            moveToMostRecent(it->second);
            value = it->second->getValue();
            return true;
        }
        return false;
    }
    Value get(Key key) override{
        Value value{};
        get(key,value);
        return value;
    }
    void remove(Key key){
        lock_guard<mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if(it != nodeMap_.end()){
            removeNode(it->second);
            nodeMap_.erase(it);
        }
    }
private:
    int     capacity_;
    NodeMap nodeMap_;
    mutex   mutex_;
    NodePtr dummyHead_;
    NodePtr dummyTail_;

    void initializeList(){
        dummyHead_ = make_shared<LruNodeType>(Key(),Value());
        dummyTail_ = make_shared<LruNodeType>(Key(),Value());
        dummyHead_->next_ = dummyTail_;
        dummyTail_->prev_ = dummyHead_;
    }
    void updateExistingNode(NodePtr node, const Value& value){
        node->setValue(value);
        moveToMostRecent(node);
    }
    void addNewNode(const Key& key, const Value& value){
        if(nodeMap_.size() >= capacity_){
            evictLeastRecent();
        }
        NodePtr newNode = make_shared<LruNodeType>(key, value);
        insertNode(newNode);
        nodeMap_[key] = newNode;
    }
    void moveToMostRecent(NodePtr node){
        removeNode(node);
        insertNode(node);
    }
    void removeNode(NodePtr node){
        if(!node->prev_.expired() && node->next_){
            auto prev = node->prev_.lock();
            prev->next_ = node->next_;
            node->next_->prev_ = prev;
            node->next_ = nullptr;
        }
    }
    void insertNode(NodePtr node){
        node->next_ = dummyTail_;
        node->prev_ = dummyTail_->prev_;
        dummyTail_->prev_.lock()->next_ = node;
        dummyTail_->prev_ = node;
    }
    void evictLeastRecent(){
        NodePtr leastRecent = dummyHead_->next_;
        removeNode(leastRecent);
        nodeMap_.erase(leastRecent->getKey());
    }
};


