/*Developed by myself and with Barisa'a help*/

#include <cassert>
#include <iostream>
#include <new>
#include <memory>

template<class T>
T* placement_copy_range(T* in_first, T* in_last, T* out_first)
{
  for(; in_first != in_last; ++ in_first, ++out_first){
      new (out_first) T(*in_first);
  }
  return out_first;
}

// preconditions:
//  [in_first, in_last) is range of initialized values
//  out_first is begining of uninitialized memory
// postcondition: uninitializes [in_first, in_last)
template<class T>
T* placement_move_range(T* in_first, T* in_last, T* out_first)
{
  for(; in_first != in_last; ++ in_first, ++out_first){
      new (out_first) T(std::move(*in_first));
      in_first->~T();
  }
  return out_first;
}


constexpr int INITIAL_STACK_SIZE = 10;

template<class T, class Allocator = std::allocator<T> >
class Stack
{
public:
    using iterator = T*;
    using const_iterator = T const*;
public:
    Stack() = default;

    ~Stack()
    {
        deInit();
    }

    Stack(const Stack& s)
        : m_first {Allocator{}.allocate(s.capacity())}
        , m_top {m_first}
        , m_last {m_first + s.capacity()}
    {
       m_top = placement_copy_range(s.m_first, s.m_top, this->m_first);

    }

    Stack(Stack&& s)
        : m_first{s.m_first}
        , m_top{s.m_top}
        , m_last{s.m_last}
    {
        s.m_first = nullptr;
        s.m_top   = nullptr;
        s.m_last  = nullptr;
    }


    Stack& operator = (Stack s){
        if (this == &s) return *this;
        deInit();

        m_first = Allocator{}.allocate(s.capacity());
        m_top = placement_copy_range(s.m_first, s.m_top, this->m_first);
        m_last = m_first + s.capacity();

        return *this;

    }

    Stack& operator = (Stack&& s){
        if (this == &s) return *this;
        deInit();

        m_first = s.m_first;
        m_top = s.m_top;
        m_last = s.m_last;

        s.m_first = nullptr;
        s.m_top = nullptr;
        s.m_last = nullptr;

        return *this;
    }

    bool empty() const { return m_first == m_top; }

    std::size_t size() const { return std::distance(m_first, m_top); }

    std::size_t capacity() const { return std::distance(m_first, m_last); }

    iterator begin()
    {
        return m_first;
    }

    iterator end()
    {
        return m_top;
    }

    const_iterator begin() const
    {
        return m_first;
    }

    const_iterator end() const
    {
        return m_top;
    }

    T& top()
    {
        assert( !empty() );
        return *(m_top - 1);
    }

    T const& top() const
    {
        assert( !empty() );
        return *(m_top - 1);
    }

    void pop()
    {
        assert( !empty() );
        -- m_top;
        m_top->~T();
    }

    void push(T t)
    {
        if(m_top == m_last)
            extend();

        new (m_top) T(t);
        ++ m_top;
    }

private:
    void extend()
    {
        assert( m_top == m_last );

        auto const new_capacity = capacity() + 5;
        T* new_first = Allocator{}.allocate(new_capacity);
        T* new_top   = placement_move_range(m_first, m_top, new_first);
        T* new_last  = new_first + new_capacity;

        if(m_first)
            Allocator{}.deallocate(m_first, capacity());

        m_first = new_first;
        m_top = new_top;
        m_last = new_last;

    }

    void deInit()
    {
        if(!m_first) return;

        for(auto iter = m_first; iter != m_top; ++iter)
        {
            iter->~T(); //destructing all the initialized elements on the stack
        }

        Allocator{}.deallocate(m_first, capacity());
        m_first = nullptr;
        m_top = nullptr;
        m_last = nullptr;
    }

    T* m_first = nullptr;
    T* m_top   = nullptr;
    T* m_last  = nullptr;
};

void test_1()
{
    /*testing basic stack functionalities*/

    Stack<int> st;

    for(int i = 0; i < 15; ++ i)
        st.push(i);

    for(int i = 14; i >= 0; -- i){
        assert( i == st.top() );
        st.pop();
    }

    assert( st.empty() );
}

void test_2 () {

    /*testing copy constructor performance*/

    Stack<int> stack4copy;
    for(int i = 0; i < 15; ++ i)
        stack4copy.push(i);

    Stack<int> st1 = stack4copy;

    for(int i = 14; i >= 0; -- i){
        assert( i == st1.top() );
        st1.pop();
    }

    assert( st1.empty() );
}

void test_3() {
    Stack<int> stack4move;

    for(int i = 0; i < 15; ++ i)
        stack4move.push(i);
    Stack<int> st1 = std::move(stack4move);

    for(int i = 14; i >= 0; -- i){
        assert( i == st1.top() );
        st1.pop();
    }

    assert( st1.empty() );

}

int main()
{
    test_1();
    test_2();
    test_3();

    return 0;
}

