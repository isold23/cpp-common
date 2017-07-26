#ifndef _WZY_LIST_H_
#define _WZY_LIST_H_

#include <cassert>

template <class T>
class _Node
{
public:
    T _data;
    _Node<T>* _link;
};
template <class T>
class _List
{
public:
    _List()
    {
        first = last = NULL; /*num = 0;*/
    }
    ~_List()
    {
        _Node<T>* next = NULL;
        
        while(first) {
            next = first->_link;
            delete first;
            first = next;
        }
        
        assert(first == NULL);
    }
    bool IsEmpty()
    {
        return NULL == first;
    }
    bool PushBack(const T& value)
    {
        _Node<T>* node = new _Node<T>;
        node->_data = value;
        node->_link = NULL;
        
        if(NULL == first) {
            first = last = node;
        } else {
            last->_link = node;
            last = node;
        }
        
        //num++;
        return true;
    }
    bool PopFront(T& value)
    {
        if(NULL == first)
            return false;
            
        _Node<T>* node = first;
        value = node->_data;
        first = node->_link;
        delete node;
        node = NULL;
        
        if(NULL == first) {
            last = NULL;
        }
        
        //num--;
        return true;
    }
    
    bool TryPop(T& value)
    {
        if(NULL == first)
            return false;
            
        value = first->_data;
        return true;
    }
    
private:
    _Node<T>* first;
    _Node<T>* last;
};
#endif //_WZY_LIST_H_
