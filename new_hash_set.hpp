#pragma once

#include <cassert>
#include "Types.hpp"
#include "Hashing.hpp"


namespace sp {
template<typename KeyType, typename ValueType>
class MyHashSet {
public:
    static constexpr double load_fact = 0.5;
    static constexpr bool debug = false;
    constexpr MyHashSet() = default;
    constexpr MyHashSet(size_t p_cap) {
        assert(p_cap != 0);
        rehash(p_cap);
    }

    ~MyHashSet() {
        delete [] store_;
        store_ = nullptr;
    }
    
    bool add(const std::pair<KeyType, ValueType>& p_ele) {
        if(size_ >= static_cast<size_t>(load_fact * capacity_)) {
            rehash(capacity_ * 2);
        }

        std::size_t insert_hash = 0;
        if(find(p_ele.first, insert_hash)) {
            return false;
        }
        Node* node = (store_ + insert_hash);
        node->ele = p_ele;
        node->state = Node::State::Allocated;
        size_++;
        return true;
    }
    
    void remove(KeyType p_key) {
        std::size_t index = 0;
        if(!find(p_key, index)) {
            return;
        }
        Node* node = (store_ + index);
        node->state = Node::State::Deallocated;
        size_--;
        return; 
    }
    
    bool contains(KeyType p_key) {
        std::size_t index = 0;
        return find(p_key, index);
    }

    ValueType& operator[](const KeyType& p_key) {
        if(size_ >= (load_fact * capacity_)) {
            rehash(capacity_ * 2);
        }

        std::size_t insert_hash = 0;
        if(find(p_key, insert_hash)) {
            Node* node = (store_ + insert_hash);
            return node->ele.second;
        }

        Node* node = (store_ + insert_hash);
        node->ele.first = p_key;
        node->state = Node::State::Allocated;
        size_++;
        if(debug) std::cout << " added " << p_key << ", size=" << size_ << '\n';
        return node->ele.second; 
    }

    bool find(KeyType p_key, std::size_t& index) {
        if(capacity_ == 0)
            return false;
        
        //auto hash = sp::MultiplicationHash(p_key, capacity_);
        auto hash = sp::Fnv1aHash(p_key) % capacity_;//, capacity_);
        //if(debug) std::cout << "for key:" << p_key << " has is :" << hash << '\n';
        index = hash;
        
        if(hash >= capacity_)
            return false;

        Node* ele = (store_ + hash);
        Node* orign = ele;
        while(ele->state != Node::State::UnAllocated) {
            if(ele->ele.first == p_key && ele->state == Node::State::Allocated) {
                index = hash;
                //assert(index != 0);
                assert(index < capacity_);
                return true;
            }
            hash = (hash + 1) % capacity_; 
            ele = store_ + hash;
            index = hash;
            if(ele == orign)
                break;
        }
        
        //assert(index != 0);
        assert(index < capacity_);

        return false;
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity(); }

private:
    struct Node {
        enum State {
            UnAllocated,
            Allocated,
            Deallocated,
        };
    
        State state = State::UnAllocated;
        std::pair<KeyType,ValueType> ele {};

        Node() = default;
        Node(std::pair<KeyType, ValueType> p_el) 
            : ele(p_el)
            , state(State::UnAllocated) 
            {}
    };

    Node* store_ = nullptr;
    sp::size_t size_ = 0;
    sp::size_t capacity_ = 0;

    constexpr void rehash(sp::size_t new_cap) {
        if(new_cap == 0)
            new_cap = 2;
        Node* new_store = (Node*) new Node[new_cap];
        if(!store_) {
            store_ = new_store;
            capacity_ = new_cap;
            size_ = 0;
            if(debug) std::cout << "Create hashtable of to size " << new_cap << '\n';
            return;
        }
        
        for(sp::size_t i = 0; i < capacity_; ++i) {
            Node* old_ele = (store_ + i);
            if(old_ele->state == Node::State::Allocated) {
                //std::size_t new_hash = sp::MultiplicationHash(old_ele->ele.first, new_cap);
                auto new_hash = sp::Fnv1aHash(old_ele->ele.first) % capacity_;
                Node* new_node = (new_store + new_hash);
                while(new_node->state != Node::State::UnAllocated) {
                    new_hash = (new_hash + 1) % new_cap;
                    new_node = new_store + new_hash;
                }
                new_node->ele = old_ele->ele;
                new_node->state = Node::State::Allocated;
                continue;
            }
        }
        
        delete  [] store_;
        capacity_ = new_cap;
        store_ = new_store;
        if(debug) std::cout << "rehash to size " << new_cap << '\n';
    }

public:
    struct Iterator {
        Iterator() = delete;
        Iterator(const MyHashSet& set, bool end = false) 
            : set_(set)
            , indx_(__INT_MAX__) {
            if(end)
                return;

            indx_ = 0;
            Node* node = set_.store_;
            while(node->state != Node::State::Allocated) {
                node++;
                indx_ = (indx_+1) % set_.capacity_;
                if(indx_ == 0)
                {
                    indx_ = __INT_MAX__;
                    break;
                }
            }
        }

        bool operator!=(const Iterator& p_rhs) const {
            return indx_ != p_rhs.indx_;
        }

        auto& operator*() {
            Node* node = set_.store_ + indx_;
            return node->ele;
        }

        auto* operator->() {
            Node* node = set_.store_ + indx_;
            return &node->ele;
        }

        Iterator& operator++() {
            indx_ = (indx_ + 1);
            Node* node = set_.store_ + indx_;

            while((indx_ != set_.capacity_) && (node->state != Node::State::Allocated) ) {
                indx_ = (indx_ + 1);
                node = set_.store_ + indx_;            
            }

            if(indx_ == set_.capacity_) {
                indx_ = __INT_MAX__;
                if(debug) std::cout << "End reached " << '\n';
            }

            return *this;
        }

        /*Iterator& operator--() {
            auto orig = indx_;
            indx_ = (indx_ - 1) % capacity_;            
            Node* node = set.store_ + indx_;

            while((indx_ != orig) && (node->state != Node::State::Allocated) ) {
                indx_ = (indx_ + 1) % capacity_;
                node = set.store_ + indx_;            
            }
            return *this;
        }*/

        const MyHashSet& set_;
        size_t indx_ = set_.capacity_;
    };

public:
    Iterator begin() {
        return Iterator(*this);
    }

    Iterator end() {
        return Iterator(*this, true);
    }
};
}//sp