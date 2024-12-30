#ifndef SHARED_PTR
#define SHARED_PTR

#include<functional>
#include <utility>

struct ptr_cnt{
    int scnt = 1;
    int wcnt = 0;
};

template <typename T>
class SharedPtr
{
public:
    T* ptr;
    ptr_cnt* num;

    void clear(){
        if(ptr && --num->scnt == 0){
            delete ptr;
            if(num){
                if(num->wcnt == 0) delete num;
            }
        }            
        ptr = nullptr;
        num = nullptr;
    }
    explicit SharedPtr(){
        ptr = nullptr;
        num = nullptr;
    }
    explicit SharedPtr(T* p){
        ptr = p;
        if(ptr != nullptr){
            num = new ptr_cnt();
        }
        else num = nullptr;
    }
    explicit SharedPtr(T* p, ptr_cnt* num1){
        ptr = p;
        num = num1;
    }
    SharedPtr(const SharedPtr& other){
        ptr = other.ptr;
        num = other.num;
        if(num) num->scnt++;
    }    
    ~SharedPtr(){
        clear();
    }

    SharedPtr& operator=(const SharedPtr& other){
        if(this != &other){
            clear();
            ptr = other.ptr;
            num = other.num;
            if(num) num->scnt++;
        }
        return *this;
    }
    SharedPtr(SharedPtr&& other) noexcept{
        ptr = other.ptr;
        num = other.num;
        other.ptr = nullptr;
        other.num = nullptr;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            clear();
            ptr = other.ptr;
            num = other.num;
            other.ptr = nullptr;
            other.num = nullptr;
        }
        return *this;
    }
    operator bool() const{
        return (ptr != nullptr && num != nullptr && num->scnt > 0);
    }
    T* get() const{
        return ptr;
    }
    int use_count() const{
        if(num) return num->scnt;
        return 0;
    }
    T& operator*() const{
        return *ptr;
    }
    T* operator->() const{
        return ptr;
    }
    void reset(){
        clear();
    }
    void reset(T* p){
        clear();
        if(p){
            ptr = p;
            num = new ptr_cnt();
        }
    }
};

template <typename T, typename... Tlist>
SharedPtr<T> make_shared(Tlist&&... rest) {
    return SharedPtr<T>(new T(std::forward<Tlist>(rest)...));
}

template <typename T>
class WeakPtr {
public:
    T* ptr;
    ptr_cnt* num;
    void clear(){
        if(num && --num->wcnt == 0 && num->scnt == 0){
            delete num;
        }
        ptr = nullptr;
        num = nullptr;
    }
    // Constructors
    WeakPtr(){
        ptr = nullptr;
        num = nullptr;
    }
    // Default constructor
    WeakPtr(const WeakPtr& other) noexcept{
        ptr = other.ptr;
        num = other.num;
        if(num) num->wcnt++;
    }  // Copy constructor
    WeakPtr(WeakPtr&& other) noexcept{
        ptr = other.ptr;
        num = other.num;
        other.ptr = nullptr;
        other.num = nullptr;
    }  // Move constructor
    WeakPtr(const SharedPtr<T>& other) noexcept{
        ptr = other.ptr;
        num = other.num;
        if(num) num->wcnt++;
    }  // Construct from SharedPtr
    
    // Destructor
    ~WeakPtr(){
        clear();
    }
    
    // Assignment operators
    WeakPtr& operator=(const WeakPtr& other){
        if(this != &other){
            clear();
            ptr = other.ptr;
            num = other.num;
            if(num) num->wcnt++;
        }
        return *this;
    }  // Copy assignment
    WeakPtr& operator=(WeakPtr&& other) noexcept{
        if(this != &other){
            clear();
            ptr = other.ptr;
            num = other.num;
            other.ptr = nullptr;
            other.num = nullptr;
        }
        return *this;
    }  // Move assignment
    WeakPtr& operator=(const SharedPtr<T>& other){
        clear();
        ptr = other.ptr;
        num = other.num;
        if(num) num->wcnt++;
        return *this;
    }  // SharedPtr assignment
    
    // Basic operations
    operator bool() const{
        return ptr != nullptr;
    }
    bool expired() const{
        return (num == nullptr || num->scnt == 0 || ptr == nullptr);
    }
    int use_count() const{
        if(num) return num->scnt;
        return 0;
    }  // Get the number of shared owners
    void reset(){
        clear();
    }
    SharedPtr<T> lock() noexcept{
        if(expired()) return SharedPtr<T>();
        if(num) num->scnt++;
        return SharedPtr<T>(ptr, num);
    }  // Get a SharedPtr to the managed object
    
    // Utility functions
    void swap(WeakPtr& other) noexcept{
        std::swap(other.ptr, ptr);
        std::swap(other.num, num);
    }  // Swap with another WeakPtr
};

// Non-member swap function
template <typename T>
void swap(WeakPtr<T>& lhs, WeakPtr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

#endif //SHARED_PTR