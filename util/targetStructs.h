/** Convenient for storing functions located inside of the targe process
 * contains common structs found in C/C++ programs,
*/

struct TargetString {
    union {
        char inline_buffer[16]; 
        char* heap_pointer;     
    };
    size_t length;              
    size_t capacity;            
};

template <typename T>
struct TargetVector {
    T* start;
    T* end;
    T* end_capacity; 
};
