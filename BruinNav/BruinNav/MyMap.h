// MyMap.h

// Skeleton for the MyMap class template.  You must implement the first six
// member functions.

#ifndef MyMap_INCLUDED
#define MyMap_INCLUDED

#include "support.h"

template<typename KeyType, typename ValueType>
class MyMap
{
public:
    MyMap():m_root(nullptr) {}
	~MyMap();
	void clear();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const MyMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	MyMap(const MyMap&) = delete;
	MyMap& operator=(const MyMap&) = delete;

private:
    struct Node {
        Node (const KeyType &key, const ValueType &value):m_key(key), m_value(value), m_left(nullptr), m_right(nullptr) {}
        KeyType m_key;
        ValueType m_value;
        Node *m_left, *m_right;
    };
    
    Node *m_root;
    
    int size(Node *node) const;
    void clear(Node *node);
};

template<typename KeyType, typename ValueType>
MyMap<KeyType, ValueType>::~MyMap() {clear(m_root);}

template<typename KeyType, typename ValueType>
void MyMap<KeyType, ValueType>::clear() {clear(m_root); m_root = nullptr;}

template<typename KeyType, typename ValueType>
void MyMap<KeyType, ValueType>::clear(Node *node) {
    if (node == nullptr) {
        return;
    }
    clear(node->m_left);
    clear(node->m_right);
    delete node;
}

template<typename KeyType, typename ValueType>
int MyMap<KeyType, ValueType>::size() const {return size(m_root);}

template<typename KeyType, typename ValueType>
int MyMap<KeyType, ValueType>::size(Node *node) const {
    if (node == nullptr) {
        return 0;
    } else {
        return(size(node->m_left) + size(node->m_right) + 1);
    }
}

template<typename KeyType, typename ValueType>
void MyMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value) {
    if (m_root == nullptr) {m_root = new Node(key, value); return;}
    Node *cur = m_root;
    for (;;) {
        if (key == cur->m_key) {
            cur->m_value = value;
            return;
        } else if (key < cur->m_key) {
            if (cur->m_left != nullptr) {
                cur = cur->m_left;
            } else {
                cur->m_left = new Node(key, value);
                return;
            }
        } else {
            if (cur->m_right != nullptr) {
                cur = cur->m_right;
            } else {
                cur->m_right = new Node(key, value);
                return;
            }
        }
    }
}

template<typename KeyType, typename ValueType>
const ValueType* MyMap<KeyType, ValueType>::find(const KeyType& key) const {
    Node *ptr = m_root;
    while (ptr != nullptr) {
        if (key == ptr->m_key){
            return &(ptr->m_value);
        } else if (key < ptr->m_key) {
            ptr = ptr->m_left;
        } else {
            ptr = ptr->m_right;
        }
    }
    return nullptr;
}

#endif
