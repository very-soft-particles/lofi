// =====================================================================================
//
//       Filename:  MemoryPool.hpp
//
//    Description: 
//
//
//        Version:  1.0
//        Created:  2023-03-15 9:55:45 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

// my includes

#include "l_base.hpp"
#include "l_vocab.hpp"
#include "l_tuple.hpp"

#define ALIGNMENT 64

namespace lofi {

  template<size_t Size>
  struct choose_index_type {
    using type = typename meta::index_type_table<meta::get_type_table_index<Size>>::type;
  };

  template<typename T>
  struct index_type_max;

  template<>
  struct index_type_max<u8> : Value<u8, MAX_u8> {};

  template<>
  struct index_type_max<u16> : Value<u16, MAX_u16> {};

  template<>
  struct index_type_max<u32> : Value<u32, MAX_u32> {};

  template<>
  struct index_type_max<u64> : Value<u64, MAX_u64> {};

  namespace mem {

    namespace helpers {

//      static const b8 is_allocated();
//
//      static void* get_tls(size_t thread_id);
//
//      b8 allocate_tls(size_t num_threads, size_t n); 
//        
//      void deallocate_tls(size_t num_threads); 
               
      template<size_t Align>
      static inline void* align_ptr(void* ptr) {
        static_assert((Align & (Align - 1)) == 0, "ptr alignment failure... alignment is not a power of 2");
        auto new_ptr = PTR2INT((u8*)ptr);
        return INT2PTR(ALIGN_POW2(PTR2INT(ptr), Align));
      }
    }		// -----  end of namespace helpers  ----- 

//    static b8 allocate_thread_storage(size_t num_threads, size_t size) {
//      if(helpers::allocate_tls(num_threads, size))
//        return true;
//      return false;
//    }
    
//    template<size_t ThreadID>
//    class TLSAllocPolicy {
//    public:
//      static void* allocate();
//      static void deallocate();
//      static void* data();
//    private:
//      static void* do_align(void* ptr);
//      static void* do_unalign(void* ptr);
//    };
   
    template<size_t N, size_t Align = 64>
    class MAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      ~MAllocPolicy();
      static constexpr size_t get_size();
      void* allocate();
      void* move(void* ptr);
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      void* data_ptr = nullptr;
      static void* do_align(void* ptr);
      static void* do_unalign(void* ptr);
    };

    template<size_t N, size_t Align = 64>
    class StackAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      using byte_t = uint8_t;
      static constexpr size_t get_size();
      void* allocate();
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      byte_t data_ptr[N + Align];
    };


    template<size_t N, size_t Align = 64>
    class SubAllocPolicy {
    public:
      using index_t = typename choose_index_type<N>::type;
      SubAllocPolicy() = default;
      SubAllocPolicy(void* ptr);
      static constexpr size_t get_size();
      void* allocate();
      void* allocate(void* ptr);
      void* move(void* ptr);
      void deallocate();
      void* data() const;
      bool belongs(void* ptr);
    private:
      void* data_ptr = nullptr;
    };

    template<size_t N, size_t Align>
    MAllocPolicy<N, Align>::~MAllocPolicy(){
      deallocate();
    }

    template<size_t N, size_t Align>
    void MAllocPolicy<N, Align>::deallocate(){
      if(data_ptr) {
        data_ptr = (void*)do_unalign(data_ptr);
        free(data_ptr);
      }
      data_ptr = nullptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::allocate() {
        //PRINT_S("entering MAlloc allocate in malloc\n");
      if(!data_ptr) {
        //PRINT_S("allocating in malloc\n");
        data_ptr = malloc(N + Align);
        if(data_ptr) {
          //PRINT_S("successfully allocated\n");
          data_ptr = do_align(data_ptr);
          return data_ptr;
        }
        //PRINT_S("failed to allocate in MAllocPolicy\n");
        return nullptr;
      }
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::move(void* ptr) {
      data_ptr = do_align(ptr);
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::data() const {
      return data_ptr;
    }

    template<size_t N, size_t Align>
    constexpr size_t MAllocPolicy<N, Align>::get_size() {
      return N;
    }

    template<size_t N, size_t Align>
    bool MAllocPolicy<N, Align>::belongs(void* ptr) {
      if(ptr >= data() && ptr < (uint8_t*)data() + N + Align)
        return true;
      return false;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::do_align(void* ptr) {
      u8* align_ptr = (u8*)helpers::align_ptr<Align>(ptr);
      u64 ptr_diff = PTR2INT(align_ptr - (u8*)ptr);
      L_ASSERT(ptr_diff < 256);
      if(!ptr_diff) {
        align_ptr += Align;
      }
      u8* new_ptr = align_ptr - 1;
      *new_ptr = (u8)ptr_diff;
      return (void*)align_ptr;
    }

    template<size_t N, size_t Align>
    void* MAllocPolicy<N, Align>::do_unalign(void* ptr) {
      u8* ptr_cast = (u8*)ptr;
      u8* new_ptr = ptr_cast - 1;
      u8 ptr_diff = (*new_ptr);
      return (void*)(ptr_cast - ptr_diff);
    }

//    template<size_t ThreadID>
//    void* TLSAllocPolicy<ThreadID>::allocate() {
//      if(helpers::is_allocated()) [[likely]] {
//        return helpers::get_tls(ThreadID);
//      }
//      return nullptr;
//    }
//
//    template<size_t ThreadID>
//    void TLSAllocPolicy<ThreadID>::deallocate(){
//      if(helpers::get_tls(ThreadID) != nullptr)
//        helpers::deallocate_tls(ThreadID);
//    }
//
//
//    template<size_t ThreadID>
//    void* TLSAllocPolicy<ThreadID>::data() {
//      return helpers::get_tls(ThreadID);
//    }

    template<size_t N, size_t Align>
    void* StackAllocPolicy<N, Align>::allocate() {
      return helpers::align_ptr<Align>(data_ptr);
    }

    template<size_t N, size_t Align>
    void StackAllocPolicy<N, Align>::deallocate() {}

    template<size_t N, size_t Align>
    constexpr size_t StackAllocPolicy<N, Align>::get_size() {
      return N;
    }

    template<size_t N, size_t Align>
    void* StackAllocPolicy<N, Align>::data() const {
      return helpers::align_ptr<Align>((void*)data_ptr);
    }

    template<size_t N, size_t Align>
    bool StackAllocPolicy<N, Align>::belongs(void* ptr) {
      if(ptr >= &data_ptr[0] && ptr < &data_ptr[N + Align]) {
        return true;
      }
      return false;
    }

    template<size_t N, size_t Align>
    SubAllocPolicy<N, Align>::SubAllocPolicy(void* ptr) 
      : data_ptr(helpers::align_ptr<Align>(ptr)) {}

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::allocate() {
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::allocate(void* ptr) {
      if(!data_ptr)
        data_ptr = helpers::align_ptr<Align>((void*)ptr);
      return data_ptr;
    }

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::move(void* ptr) {
      data_ptr = helpers::align_ptr<Align>((void*)ptr);
      return data_ptr;
    }


    template<size_t N, size_t Align>
    void SubAllocPolicy<N, Align>::deallocate() {}

    template<size_t N, size_t Align>
    constexpr size_t SubAllocPolicy<N, Align>::get_size() {
      return N;
    }

    template<size_t N, size_t Align>
    void* SubAllocPolicy<N, Align>::data() const {
      return data_ptr;
    }

    template<size_t N, size_t Align>
    bool SubAllocPolicy<N, Align>::belongs(void* ptr) {
      if(!data_ptr)
        return false;
      if(ptr >= data() && ptr < (uint8_t*)data() + N)
        return true;
      return false;
    }

    //template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy>
    //class PackedRingBufferContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
    //public:
    //  using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
    //  using alloc_t::data;
    //  using index_t = typename alloc_t::index_t;
    //  using alloc_ptr_t = typename alloc_t::ptr_t;
    //  using value_t = T;
    //  using ptr_t = T*;
    //  using ref_t = T&;

    //  static_assert((Size & (Size - 1)) == 0, "size of ring buffer must be a power of two");
    //  
    //  static constexpr index_t mask_value = Size - 1;
    //  static constexpr index_t null_value = index_type_max<index_t>::value;

    //  PackedRingBufferContainerPolicy() {
    //    alloc_t::allocate();
    //  }

    //  ~PackedRingBufferContainerPolicy() {}

    //  T& operator[](const index_t index) {
    //    return *get_ptr_cast(index);
    //  }

    //  b8 push(T src) {
    //    if(tail == null_value) {
    //      head = tail = 0;
    //    } else if (tail >= Size) {
    //      tail &= mask_value;
    //      if(tail >= head) {
    //        return false;
    //      }
    //    }
    //    *get_ptr_cast(tail++) = src;
    //    return true;
    //  }

    //  b8 push(T* src) {
    //    if(tail == null_value) {
    //      head = tail = 0;
    //    } else if (tail >= Size) {
    //      tail &= mask_value;
    //      if(tail >= head) {
    //        return false;
    //      }
    //    }
    //    MEM_COPY(get_ptr_cast(tail++), src, sizeof(T));
    //    return true;
    //  }

    //  b8 push(T& src) {
    //    if(tail == null_value) {
    //      head = tail = 0;
    //    } else if (tail >= Size) {
    //      tail &= mask_value;
    //      if(tail >= head) {
    //        return false;
    //      }
    //    }
    //    *get_ptr_cast(tail++) = src;
    //    return true;
    //  }

    //  b8 push(T&& src) {
    //    if(tail == null_value) {
    //      head = tail = 0;
    //    } else if (tail >= Size) {
    //      tail &= mask_value;
    //      if(tail >= head) {
    //        return false;
    //      }
    //    }
    //    *get_ptr_cast(tail++) = src;
    //    return true;
    //  }

    //  b8 pop(ptr_t dst) {
    //    if(head == null_value) {
    //      return false;
    //    }
    //    if(head >= Size) {
    //      head &= mask_value;
    //    }
    //    if(head >= tail) {
    //      return false;
    //    }
    //    MEM_COPY(dst, get_ptr_cast(head++), sizeof(T));
    //    return true;
    //  }

    //  b8 remove_object(const index_t index) {
    //    if(tail <= head) {
    //      if(index >= head || index < tail) {
    //        ptr_t dst = get_ptr_cast(index);
    //        ptr_t src = get_ptr_cast(tail--);
    //        MEM_COPY(dst, src, sizeof(T));
    //        return true;
    //      }
    //      return false;
    //    }
    //    if(index >= head && index < tail) {
    //      ptr_t dst = get_ptr_cast(index);
    //      ptr_t src = get_ptr_cast(tail--);
    //      MEM_COPY(dst, src, sizeof(T));
    //      return true;
    //    }
    //    return false;
    //  }

    //  index_t get_head() {
    //    return head;
    //  }

    //  index_t get_tail() {
    //    return tail;
    //  }
    //private:

    //  ptr_t get_ptr_cast(const index_t index) {
    //    return (ptr_t)data() + index;
    //  }

    //  index_t head = null_value;
    //  index_t tail = null_value;
    //};

    template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy>
    class PackedArrayContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
      using alloc_t::data;
      using index_t = typename alloc_t::index_t;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
  
      PackedArrayContainerPolicy() {
        alloc_t::allocate();
      }

      ~PackedArrayContainerPolicy() {}
  
      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
      
      index_t add_object(T&& obj) {
        L_ASSERT(top != Size);
        static_cast<T*>(alloc_t::data())[top] = obj;
        return top++;
      }
  
      void remove_object(index_t index) {
        static_cast<T*>(alloc_t::data())[index] = static_cast<T*>(alloc_t::data())[--top];
      }
  
      void clear() {
        top = 0;
      }
  
    private:
      index_t top = 0;
    };

    template<class T, size_t Size, size_t Alignment>
    class PackedArrayContainerPolicy<T, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = SubAllocPolicy<Size * sizeof(T), Alignment>;
      using index_t = typename alloc_t::index_t;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
 
      PackedArrayContainerPolicy() = delete;
      PackedArrayContainerPolicy(void* ptr) : alloc_t(ptr) {}

      ~PackedArrayContainerPolicy() {}
  
      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }

      void move_ptr(void* ptr) {
        clear();
        this->allocate(ptr);
      }
      
      size_t add_object(T obj) {
        L_ASSERT(top != Size);
        MEM_COPY(static_cast<T*>(alloc_t::data()) + top, &obj, sizeof(T));
        return top++;
      }
  
      void remove_object(size_t index) {
        static_cast<T*>(alloc_t::data())[index] = static_cast<T*>(alloc_t::data())[--top];
      }
  
      void clear() {
        top = 0;
      }

      const size_t get_size() {
        return top;
      }
  
    private:
      index_t top = 0;
    };

    template<class T, size_t Size, size_t Alignment, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
    class SparseArrayContainerPolicy : AllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = AllocPolicy<Size * sizeof(T), Alignment>;
      using alloc_t::data;
      using index_t = typename alloc_t::index_t;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
  
      SparseArrayContainerPolicy() {
        static_assert(sizeof(T) >= sizeof(uint32_t), "Size too small for T in SparseArray");
        alloc_t::allocate(sizeof(T) * Size);
        reset_indices();
      }
  
      ~SparseArrayContainerPolicy() {}

      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
 
      index_t add_object(T obj) {
        L_ASSERT(top != UINT32_MAX);
        auto here = *get_handle(top);
        auto temp = *get_handle(here);
  
        static_cast<T*>(alloc_t::data())[top] = obj;
  
        here = top;
        top = temp;
  
        return here;
      }
  
      void remove_object(const index_t index) {
        *get_handle(index) = top;
        top = index;
      }
  
      static constexpr size_t get_size() {
        return Size;
      }
  
    private:
      void reset_indices() {
        for(index_t i = 0; i < Size; i++) {
          if(i + 1 == Size) {
            *get_handle(i) = UINT32_MAX;
            break;
          }
          *get_handle(i) = i + 1;
        }
      }
  
      inline index_t* get_handle(const index_t index) {
        return ((uint32_t*)(static_cast<T*>(alloc_t::data()) + index));
      }
  
      index_t top = 0;
    };
  

    template<class T, size_t Size, size_t Alignment>
    class SparseArrayContainerPolicy<T, Size, Alignment, SubAllocPolicy> : SubAllocPolicy<Size * sizeof(T), Alignment> {
    public:
      using alloc_t = SubAllocPolicy<Size * sizeof(T), Alignment>;
      using alloc_t::data;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using index_t = typename alloc_t::index_t;


      SparseArrayContainerPolicy() = delete;
  
      SparseArrayContainerPolicy(void* ptr) : alloc_t(ptr) {
        static_assert(sizeof(T) >= sizeof(index_t), "size of T too small in SparseArray");
        reset_indices();
      }
  
      ~SparseArrayContainerPolicy() {}
 
      T& operator[](const index_t index) {
        return static_cast<T*>(alloc_t::data())[index];
      }
  
      index_t add_object(T obj) {
  
        L_ASSERT(top != UINT32_MAX);
        auto here = *get_handle(top);
        auto temp = *get_handle(here);
  
        static_cast<T*>(alloc_t::data())[top] = obj;
  
        here = top;
        top = temp;
  
        return here;
      }
  
      void remove_object(const index_t index) {
        *get_handle(index) = top;
        top = index;
      }
  
      static constexpr size_t get_size() {
        return Size;
      }
  
    private:
      void reset_indices() {
        for(size_t i = 0; i < Size; i++) {
          if(i + 1 == Size) {
            *get_handle(i) = UINT32_MAX;
            break;
          }
          *get_handle(i) = i + 1;
        }
      }
  
      inline index_t* get_handle(const index_t index) {
        return ((uint32_t*)(static_cast<T*>(alloc_t::data()) + index));
      }
  
      uint32_t top = 0;
    };
   

    template<class KeyType>
    struct choose_null_key;

    template<>
    struct choose_null_key<uint8_t> {
      static constexpr uint8_t value = MAX_u8;
    };

    template<>
    struct choose_null_key<uint16_t> {
      static constexpr uint16_t value = MAX_u16;
    };

    template<>
    struct choose_null_key<uint32_t> {
      static constexpr uint32_t value = MAX_u32;
    };

    template<>
    struct choose_null_key<uint64_t> {
      static constexpr uint64_t value = MAX_u64;
    };

    template<class T, size_t Alignment, template<size_t, size_t> class AllocPolicy, class KeyType = uint32_t, uint32_t Size = 64>
    class Map : AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment> {
    public:
      using alloc_t = AllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size, Alignment>;
      static constexpr KeyType null_key = choose_null_key<KeyType>::value;

      
      Map() {
        static_assert(((Size - 1) & (~Size)) == (Size - 1), "Size of map is not a power of two");
        reset_keys();
      }
  
      ~Map() {
      }
  
      T& operator[](const KeyType index) {
        const KeyType true_key = find_key(index);
        L_ASSERT(true_key != null_key);
        return get_values(true_key);
      }
  
      KeyType insert(const uint32_t key, const T t) {
        auto new_key = get_new_key(key);
        if(new_key == null_key)
          return null_key;
        get_keys()[new_key] = key;
        get_values()[new_key] = t;
      }
  
      void remove(const KeyType key) {
        auto new_key = find_key(key);
        set_key_to_null(new_key);
      }
  
      bool has(const uint32_t key) {
        return find_key(key) != null_key;
      }
  
    private:
      void reset_keys() {
        for(size_t i = 0; i < Size; i++) {
          set_key_to_null(i);
        }
      }
  
      KeyType find_key(const KeyType key) {
        uint32_t acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(get_keys()[acc] == key)
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return null_key;
      }
  
      KeyType get_new_key(const KeyType key) {
        KeyType acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(key_is_null(acc))
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return null_key;
      }
  
      inline bool key_is_null(const KeyType key) {
        return get_keys()[key] == null_key;
      }
  
      inline void set_key_to_null(const KeyType key) {
        get_keys()[key] = null_key;
      }
  
      inline KeyType* get_keys() {
        return static_cast<KeyType*>(alloc_t::data());
      }
  
      inline T* get_values() {
        return static_cast<T*>(get_keys() + Size);
      }
  
    };



    template<class T, size_t Alignment, class KeyType, uint32_t Size>
    class Map<T, Alignment, SubAllocPolicy, KeyType, Size> : SubAllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size> {
    public:
      using alloc_t = SubAllocPolicy<(sizeof(T) + sizeof(KeyType)) * Size>;
      static constexpr KeyType null_key = choose_null_key<KeyType>::value;

      Map() = delete;

      Map(void* ptr) : alloc_t(ptr) {
        static_assert(((Size - 1) & (~Size)) == (Size - 1), "Size of map is not a power of two");
        reset_keys();
      }
  
      ~Map() {
      }
  
      T& operator[](const KeyType index) {
        const KeyType true_key = find_key(index);
        L_ASSERT(true_key != null_key);
        return get_values(true_key);
      }
  
      uint32_t insert(const uint32_t key, const T t) {
        auto new_key = get_new_key(key);
        if(new_key == UINT32_MAX)
          return UINT32_MAX;
        get_keys()[new_key] = key;
        get_values()[new_key] = t;
      }
  
      void remove(const KeyType key) {
        auto new_key = find_key(key);
        set_key_to_null(new_key);
      }
  
      bool has(const uint32_t key) {
        return find_key(key) != UINT32_MAX;
      }
  
    private:
      void reset_keys() {
        for(size_t i = 0; i < Size; i++) {
          set_key_to_null(i);
        }
      }
  
      KeyType find_key(const KeyType key) {
        uint32_t acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(get_keys()[acc] == key)
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return UINT32_MAX;
      }
  
      KeyType get_new_key(const KeyType key) {
        KeyType acc = key & (Size - 1);
        auto count = Size;
  
        while(count--) {
          if(key_is_null(acc))
            return acc;
          ++acc;
          acc &= (Size - 1);
        }
  
        return UINT32_MAX;
      }
  
      inline bool key_is_null(const KeyType key) {
        return get_keys()[key] == UINT32_MAX;
      }
  
      inline void set_key_to_null(const KeyType key) {
        get_keys()[key] = UINT32_MAX;
      }
  
      inline KeyType* get_keys() {
        return static_cast<KeyType*>(alloc_t::data());
      }
  
      inline T* get_values() {
        return static_cast<T*>(get_keys() + Size);
      }
  
    };
 
    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
    class DoubleBufferContainerPolicy : private AllocPolicy<N * sizeof(T) * 2, Align> {
    public:
      using alloc_t = AllocPolicy<N * sizeof(T), Align>;
      using alloc_t::data;
      using alloc_t::belongs;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using index_t = typename choose_index_type<N * 2>::type;
     
      DoubleBufferContainerPolicy() {
        current_write_buffer = 0;
        top[0] = bottom[0];
        top[1] = bottom[1];
      }

      ptr_t write() {
        if(top[current_write_buffer] == bottom[current_write_buffer] + HalfSize)
          return nullptr;
        return &get_ptr_cast()[top[current_write_buffer]++];
      }

      ptr_t read(index_t* count) {
        *count = top[!current_write_buffer] - bottom[!current_write_buffer];
        return &get_ptr_cast()[bottom[!current_write_buffer]];
      }

      index_t current_read_buffer_size() const {
        return top[!current_write_buffer];
      }

      void swap_buffers() {
        current_write_buffer ^= 1;
      }

      void clear_writes() {
        top[current_write_buffer] = bottom[current_write_buffer];
      }

      void clear_reads() {
        top[!current_write_buffer] = bottom[!current_write_buffer];
      }

      void clear_all() {
        top[0] = bottom[0];
        top[1] = bottom[1];
      }

    private:
      ptr_t get_ptr_cast() {
        return (ptr_t)alloc_t::data();
      }
      static constexpr index_t HalfSize = N/2;
      static constexpr index_t bottom[] {0,HalfSize};
      index_t top[2];
      index_t current_write_buffer;
    };

    template<class T, size_t N, size_t Align>
    class DoubleBufferContainerPolicy<T, N, Align, SubAllocPolicy> : private SubAllocPolicy<N * sizeof(T), Align> {
    public:
      using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;
      using alloc_t::data;
      using alloc_t::belongs;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using index_t = typename choose_index_type<N * 2>::type;
     
      DoubleBufferContainerPolicy(void* ptr) : alloc_t(ptr) {
        current_write_buffer = 0;
        top[0] = bottom[0];
        top[1] = bottom[1];
      }

      DoubleBufferContainerPolicy() : alloc_t(nullptr) {
        current_write_buffer = 0;
        top[0] = bottom[0];
        top[1] = bottom[1];
      }

      void move_ptr(void* ptr) {
        this->move(ptr);
      }

      ptr_t write() {
        if(top[current_write_buffer] == bottom[current_write_buffer] + HalfSize)
          return nullptr;
        return &get_ptr_cast()[top[current_write_buffer]++];
      }

      ptr_t read(index_t* count) {
        *count = top[!current_write_buffer] - bottom[!current_write_buffer];
        return &get_ptr_cast()[bottom[!current_write_buffer]];
      }

      index_t current_read_buffer_size() const {
        return top[!current_write_buffer];
      }

      void swap_buffers() {
        current_write_buffer ^= 1;
      }

      void clear_writes() {
        top[current_write_buffer] = bottom[current_write_buffer];
      }

      void clear_reads() {
        top[!current_write_buffer] = bottom[!current_write_buffer];
      }

      void clear_all() {
        top[0] = 0;
        top[1] = N;
      }
    private:
      ptr_t get_ptr_cast() {
        return (ptr_t)alloc_t::data();
      }
      static constexpr index_t HalfSize = N/2;
      static constexpr index_t bottom[] {0,HalfSize};
      index_t top[2];
      index_t current_write_buffer;
    };
 
    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy = SubAllocPolicy>
    class FreeListContainerPolicy : private AllocPolicy<N * sizeof(T), Align> {
    public:
      using alloc_t = AllocPolicy<N * sizeof(T), Align>;
      using alloc_t::data;
      using alloc_t::belongs;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      
      FreeListContainerPolicy() {
        reset_elements();
      }


      ~FreeListContainerPolicy(){};

      ptr_t add_object() {
        auto thisone = (T**)top;
        if(thisone == nullptr) {
          return nullptr;
        }
        auto temp = *thisone;
        top = temp;

        return (T*)thisone;
      }

      bool remove_object(void* rhs) {
        if(!alloc_t::belongs((void*)rhs))
          return false;
        ptr_t temp = top;
        top = (ptr_t)rhs;
        *((T**)top) = temp;
        return true;
      }

      T* clear() {
        reset_elements();
        return top;
      }

      static constexpr size_t get_block_size() {
        return sizeof(T);
      }

      static constexpr size_t get_block_count() {
        return N;
      }

    private:
      void reset_elements() {
        top = (ptr_t)alloc_t::data();

        for(size_t i = 0; i < N; i++)
        {
          size_t next = i + 1;
          ptr_t* temp = (T**)(top + i);
          if(next == N) {
            *temp = nullptr;
            break;
          }
          *temp = (top + next);
        }
      }
      T* top;
    };

    template<class T, size_t N, size_t Align>
    class FreeListContainerPolicy<T, N, Align, SubAllocPolicy> : private SubAllocPolicy<N * sizeof(T), Align> {
    public:
      using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;
      using alloc_t::data;
      using alloc_t::belongs;
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      FreeListContainerPolicy() : alloc_t(nullptr) {}
      FreeListContainerPolicy(void* ptr);

      void move_ptr(void* ptr) {
        top = (ptr_t)this->move(ptr);
        reset_elements();
      }

      ptr_t add_object() {
        auto thisone = (T**)top;
        if(thisone == nullptr) {
          return nullptr;
        }
        auto temp = *thisone;
        top = temp;
        return (T*)thisone;
      }

      bool remove_object(void* rhs) {
        if(!alloc_t::belongs(rhs)){
          return false;
        }
        void** temp = (void**)rhs;
        *temp = top;
        top = (ptr_t)rhs;
        return true;
      };

      static constexpr size_t get_block_size() {
        return sizeof(T);
      }

      static constexpr size_t get_block_count() {
        return N;
      }

    private:
      void reset_elements() {
        top = (ptr_t)alloc_t::data();
        for(size_t i = 0; i < N; i++)
        {
          size_t next = i + 1;
          ptr_t* temp = (T**)(top + i);
          if(next == N) {
            *temp = nullptr;
            break;
          }
          *temp = (top + next);
        }
      }
      ptr_t top = nullptr;
    };


    template<size_t N, size_t Align, template<size_t, size_t> class AllocPolicy>
    class StackContainerPolicy : AllocPolicy<N, Align> {
    public:
      using byte_t = uint8_t;
      using alloc_t = AllocPolicy<N, Align>;
      using value_t = byte_t;
      using ptr_t = byte_t*;
      using ref_t = byte_t&;
      using alloc_t::belongs;

      StackContainerPolicy() {
        top = (ptr_t)alloc_t::allocate();
      };

      ptr_t push(size_t size) {
        if(!alloc_t::belongs((typename alloc_t::ptr_t)((uint8_t*)top + size)))
          return nullptr;

        auto ret = top;
        top += size;
        top = helpers::align_ptr<Align>( top);
        return ret;
      };

      void pop_to(ptr_t ptr) {
        top = ptr;
      };

      void clear() {
      }

    private:
      ptr_t top = nullptr;

    };


    template<size_t N, size_t Align>
    class StackContainerPolicy<N, Align, SubAllocPolicy> : SubAllocPolicy<N, Align> {
    public:
      using byte_t = uint8_t;
      using alloc_t = SubAllocPolicy<N>;
      using value_t = byte_t;
      using ptr_t = byte_t*;
      using ref_t = byte_t&;

      StackContainerPolicy(ptr_t ptr);

      ptr_t push(size_t size) {
        if(!alloc_t::belongs((typename alloc_t::ptr_t)((uint8_t*)top + size)))
          return nullptr;

        auto ret = top;
        top += size;
        top = helpers::align_ptr<Align>( top);
        return ret;
      };

      void pop_to(ptr_t ptr) {
        top = ptr;
      };

    private:
      ptr_t top = nullptr;

    };

    template<class T, size_t N, size_t Align, template<size_t, size_t> class AllocPolicy>
    class ArrayContainerPolicy : AllocPolicy<N * sizeof(T), Align> {
    public:
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using index_t = typename choose_index_type<N>::type;
      using alloc_t = AllocPolicy<N * sizeof(T), Align>;

      ArrayContainerPolicy() : alloc_t() {
        alloc_t::allocate();
      }
      
      ref_t operator[](index_t index) {
        return get_array_cast()[index];
      }

      const index_t get_size() const {
        return top;
      }

      ptr_t get_buffer() {
        return get_array_cast();
      }

      ptr_t push(index_t count) {
        index_t result = top;
        index_t new_top = top + count;
        if(new_top > N) {
          return nullptr;
        }
        top = new_top;
        return get_array_cast() + result;
      }

      static constexpr index_t get_cap() {
        return (index_t)N;
      }

      void pop(index_t count) {
        index_t new_top = top - count;
        if(new_top >= top){
          return;
        }
        top = new_top;
      }

      void clear() {
        top = 0;
      }

    private:
      T* get_array_cast() {
        return (ptr_t)alloc_t::data();
      }
      index_t top = 0;
    };

    template<class T, size_t N, size_t Align>
    class ArrayContainerPolicy<T, N, Align, SubAllocPolicy> : SubAllocPolicy<N * sizeof(T)
    , Align> {
    public:
      using value_t = T;
      using ptr_t = T*;
      using ref_t = T&;
      using const_ref_t = const T&;
      using index_t = typename choose_index_type<N>::type;
      using alloc_t = SubAllocPolicy<N * sizeof(T), Align>;

      ArrayContainerPolicy() : alloc_t(nullptr) {}
      
      ArrayContainerPolicy(void* ptr) : alloc_t((ptr_t)ptr) {}

      void move_ptr(void* ptr) {
        this->move(ptr);
      }
      
      T& operator[](size_t index) {
        return get_array_cast()[index];
      }
 
      const index_t get_size() const {
        return top;
      }

      ptr_t get_buffer() {
        return get_array_cast();
      }

      ptr_t push(index_t count) {
        index_t result = top;
        index_t new_top = top + count;
        if(new_top > N) {
          return nullptr;
        }
        top = new_top;
        return get_array_cast() + result;
      }

      static constexpr index_t get_cap() {
        return (index_t)N;
      }

      void pop(index_t count) {
        index_t new_top = top - count;
        if(new_top >= top){
          return;
        }
        top = new_top;
      }

      void clear() {
        top = 0;
      }

    private:
      T* get_array_cast() {
        return (ptr_t)alloc_t::data();
      }
      index_t top = 0;
    };

    template<class T, size_t N, size_t Align>
    FreeListContainerPolicy<T, N, Align, SubAllocPolicy>::FreeListContainerPolicy(void* ptr) : SubAllocPolicy<N * sizeof(T), Align>(ptr) {
      reset_elements();
    }


    template<size_t N, size_t Align>
    StackContainerPolicy<N, Align, SubAllocPolicy>::StackContainerPolicy(ptr_t ptr) {
      top = alloc_t::allocate(ptr);
    }


// Bucket generation

    template<size_t BlockSize>
    struct Block {
      uint8_t block [BlockSize];
    };

    template<size_t BlockSize, size_t BlockCount, size_t Alignment>
    struct BucketDescriptor 
    {
      static constexpr size_t size = BlockSize;
      static constexpr size_t count = BlockCount;
      static constexpr size_t align = Alignment;
    };

    template<size_t ID>
    struct BucketDescriptorSet 
    {
      using type = List<>;
    };

    template<size_t ID>
    struct bucket_info {
      using bucket_descriptors_t = typename BucketDescriptorSet<ID>::type;

      static constexpr size_t bucket_count = list_size<bucket_descriptors_t>::value;

      template<typename Idx>
      struct block_info {
        static constexpr size_t block_size = meta::at_t<bucket_descriptors_t, Idx::value>::size;

        static constexpr size_t block_count = meta::at_t<bucket_descriptors_t, Idx::value>::count;

        static constexpr size_t block_align = meta::at_t<bucket_descriptors_t, Idx::value>::align;

        static constexpr size_t bucket_size = block_size * block_count;
      };

      template<typename... Idxs>
      using block_info_seq_t = List<block_info<Idxs>...>;

      template<typename... Idxs>
      using block_size_seq_t = List<IdxT<block_info<Idxs>::block_size>...>;

      template<typename... Idxs>
      using block_count_seq_t = List<IdxT<block_info<Idxs>::block_count>...>;

      template<typename... Idxs>
      using block_align_seq_t = List<IdxT<block_info<Idxs>::block_align>...>;

      template<typename... Idxs>
      using bucket_size_seq_t = List<IdxT<block_info<Idxs>::bucket_size + block_info<Idxs>::block_align>...>;

      using apply_block_info_seq = typename meta::lift<block_info_seq_t, typename IdxSequence<bucket_count>::type>;

      using apply_block_size_seq = typename meta::lift<block_size_seq_t, typename IdxSequence<bucket_count>::type>;

      using apply_block_count_seq = typename meta::lift<block_count_seq_t, typename IdxSequence<bucket_count>::type>;

      using apply_block_align_seq = typename meta::lift<block_align_seq_t, typename IdxSequence<bucket_count>::type>;

      using apply_bucket_size_seq = typename meta::lift<bucket_size_seq_t, typename IdxSequence<bucket_count>::type>;

      static constexpr size_t set_size = meta::lift<meta::apply_sum, typename apply_bucket_size_seq::type>::type::value;

      static constexpr size_t set_count = meta::lift<meta::apply_sum, typename apply_block_count_seq::type>::type::value; 

      static constexpr size_t set_alignment = meta::lift<meta::apply_sum, typename apply_block_align_seq::type>::type::value; 

      template<typename Info>
      using Bucket = FreeListContainerPolicy<Block<Info::block_size>, Info::block_count, Info::block_align, SubAllocPolicy>;

      template<typename... Infos>
      using bucket_zip = List<Bucket<Infos>...>;

      using apply_bucket_zip = typename meta::lift<bucket_zip, typename apply_block_info_seq::type>;
    };

    template<size_t ID>
    using make_pool = typename meta::lift<tuple_base, typename bucket_info<ID>::apply_bucket_zip::type>::type;

    template<size_t ID>
    struct set_buckets {
      using info = bucket_info<ID>;
      static constexpr size_t num_buckets = info::bucket_count;
      template<size_t Index = 0>
      struct type {
        using block_info = typename info::template block_info<IdxT<Index>>;
        static constexpr size_t size = block_info::bucket_size + block_info::block_align;
        template<template<size_t> class PoolT>
        static void apply(PoolT<ID>* pool_ptr, void* ptr) {
          //PRINT("applying set pointers, ptr at: %llu\n", ptr);
          pool_ptr->template get<Index>().move_ptr(ptr);
          u8* new_ptr = (u8*)ptr + size;
          if constexpr (Index + 1 < num_buckets) {
            type<Index + 1>::apply(pool_ptr, (void*)new_ptr);
          }
        }
      };
    };

    template<size_t ID>
    struct StaticMemoryPool : StackAllocPolicy<bucket_info<ID>::set_size, 1024>
                            , make_pool<ID> 
    {
      using base_t = make_pool<ID>;
      using list_t = typename base_t::type;
      using alloc_t = StackAllocPolicy<bucket_info<ID>::set_size, 1024>;
      StaticMemoryPool() {
        set_pointers();
      }
      template<size_t Index>
      auto& get() {
        return static_cast<meta::at_t<list_t, Index>&>(*this);
      }
    private:
      void set_pointers() {
        u8* start = (u8*)alloc_t::data();
        set_buckets<ID>::template type<>::apply(this, start);
      }
    };

    template<size_t ID>
    struct HeapMemoryPool : MAllocPolicy<bucket_info<ID>::set_size, 256>
                          , make_pool<ID>
    {
      using base_t = make_pool<ID>;
      using list_t = typename base_t::type;
      using alloc_t = MAllocPolicy<bucket_info<ID>::set_size, 256>;
      HeapMemoryPool() {
        //PRINT_S("constructing heap memory pool\n");
        alloc_t::allocate();
        //PRINT_S("setting pointers\n");
        set_pointers();
      }

      template<size_t Index>
      auto& get() {
        return static_cast<meta::at_t<list_t, Index>&>(*this);
      }
    private:
      void set_pointers() {
        u8* start = (u8*)alloc_t::data();
        set_buckets<ID>::template type<>::apply(this, start);
      }
    };

    template<size_t ID>
    auto& get_pool() {
      static StaticMemoryPool<ID> _pool{};
      return _pool;
    };
    
    namespace defaults {
      template<size_t Count>
      using Bucket_8 = BucketDescriptor<1 << 3, Count, 1 << 3>; // 16 byte aligned bucket
      template<size_t Count>
      using Bucket_16 = BucketDescriptor<1 << 4, Count, 1 << 4>; // 16 byte aligned bucket
      template<size_t Count>
      using Bucket_32 = BucketDescriptor<1 << 5, Count, 1 << 5>; // 32 byte aligned bucket
      template<size_t Count>
      using Bucket_64 = BucketDescriptor<1 << 6, Count, 1 << 6>; // 64 byte aligned bucket
      template<size_t Count>
      using Bucket_256 = BucketDescriptor<1 << 8, Count, 1 << 8>; // 256 byte aligned bucket

      template<size_t Count>
      using Bucket_1024 = BucketDescriptor<1 << 10, Count, 1 << 8>; // 1024 byte aligned bucket
      template<size_t Count>
      using Bucket_4096 = BucketDescriptor<1 << 12, Count, 1 << 8>; // 4096 byte aligned bucket
      template<size_t Count>
      using Bucket_16384 = BucketDescriptor<1 << 14, Count, 1 << 8>; // 16384 byte aligned bucket
      template<size_t Count>
      using Bucket_65536 = BucketDescriptor<1 << 16, Count, 1 << 8>; // 65536 byte aligned bucket
      template<size_t Count>
      using Bucket_256KB = BucketDescriptor<KB(256), Count, 1 << 8>; // 65536 byte aligned bucket
      template<size_t Count>
      using Bucket_1M = BucketDescriptor<MB(1), Count, 1 << 8>; // 1 mega-byte aligned bucket
      template<size_t Count>
      using Bucket_16M = BucketDescriptor<MB(16), Count, 1 << 8>; // 1 mega-byte aligned bucket
      template<size_t NumFibers>
      using FiberBucket = BucketDescriptor<KB(64) * NumFibers, 1, 1 << 8>; // 1 mega-byte aligned bucket

    }		// -----  end of namespace defaults  ----- 
  }		// -----  end of namespace mem  ----- 

  template<size_t ID>
  class HeapAllocator {
  public:
    HeapAllocator() : _pool() {}

    template<typename T>
    T* allocate() noexcept {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<N>::value;
      return (T*)FWD(_pool.template get<I>().add_object());
    }
    
    template<typename T>
    void deallocate(T* ptr) noexcept {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<N>::value;
      _pool.template get<I>().remove_object(static_cast<void*>(FWD(ptr)));
    }

    template<typename T, typename U>
    void* rebind(U* src) noexcept {
      auto new_ptr = allocate<ID, T>();
      L_ASSERT(new_ptr != nullptr);
      auto size = CLAMP_TOP(sizeof(T), sizeof(U));
      MEM_COPY(new_ptr, src, size);
      deallocate<ID, U>(src);
      return new_ptr;
    }

  private:
    mem::HeapMemoryPool<ID> _pool;


    template<size_t N>
    struct find_bucket {
      template<typename... Idxs>
      using type = typename meta::pred_find_t<meta::less_t<IdxT<N>>::template type>::template type<Idxs...>::type;
    };

    template<size_t N>
    using apply_find_bucket = typename meta::lift<find_bucket<N>::template type, typename mem::bucket_info<ID>::apply_block_size_seq::type>::type;

  };


  class StaticAllocator {
  public:
    template<size_t ID, typename T>
    static T* allocate() {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<ID, N>::value;
      return (T*)FWD(mem::get_pool<ID>().template get<I>().add_object());
    }
    
    template<size_t ID, typename T>
    static void deallocate(T* ptr) {
      static constexpr size_t N = sizeof(T);
      static constexpr size_t I = apply_find_bucket<ID, N>::value;
      mem::get_pool<ID>().template get<I>().remove_object(static_cast<void*>(FWD(ptr)));
    }

    template<size_t ID, typename T, typename U>
    static void* rebind(U* src) {
      auto new_ptr = allocate<ID, T>();
      L_ASSERT(new_ptr != nullptr);
      auto size = CLAMP_TOP(sizeof(T), sizeof(U));
      MEM_COPY(new_ptr, src, size);
      deallocate<ID, U>(src);
      return new_ptr;
    }

  private:
    template<size_t N>
    struct find_bucket {
      template<typename... Idxs>
      using type = typename meta::pred_find_t<meta::less_t<IdxT<N>>::template type>::template type<Idxs...>::type;
    };

    template<size_t ID, size_t N>
    using apply_find_bucket = typename meta::lift<find_bucket<N>::template type, typename mem::bucket_info<ID>::apply_block_size_seq::type>::type;

  };

  template<size_t ID>
  class RuntimeAllocator {
  public:
    static void* allocate(size_t size, size_t alignment = 0) {
      static constexpr auto size_array = index_array<typename mem::bucket_info<ID>::apply_block_size_seq::type>;
      size = MAX(size, alignment);
      auto func = [](size_t size) {
        for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
          if(size <= size_array[i]){
            return i;
          }
        }
        return MAX_u64;
      };

      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              return (void*)mem::get_pool<ID>()
                                .template get<Index>()
                                .add_object();
                            }};

      return _dispatcher(FWD(func(size)));
    }

    static void free(void* ptr) {
      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              if(mem::get_pool<ID>().template get<Index>().belongs(ptr)) {
                                mem::get_pool<ID>().template get<Index>().remove_object(ptr);
                                return true;
                              }
                              return false;
                            }};
      for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
        if(_dispatcher(i)) 
          return;
      }
      L_ASSERT(false);
    }

    static void* reallocate(void* ptr, size_t size, size_t alignment) {
      dispatcher _dispatcher{ IdxV<mem::bucket_info<ID>::bucket_count>
                            , [&]<size_t Index>(IdxT<Index>){
                              if(mem::get_pool<ID>().template get<Index>().belongs(ptr)) {
                                return mem::get_pool<ID>().template get<Index>().get_block_size();
                              }
                              return (size_t)0;
                            }};
      
      size = MAX(size, alignment);
      void* new_ptr = allocate(size);
      size_t old_size{};
      for(size_t i = 0; i < mem::bucket_info<ID>::bucket_count; i++) {
        old_size = _dispatcher(i);
        if(old_size)
          break;
      }
      L_ASSERT(new_ptr != nullptr);
      old_size = MIN(size, old_size);
      MEM_COPY(new_ptr, ptr, old_size);
      free(ptr);
      return new_ptr;
    } 


  };

  template<size_t InitialCap = MB(1), size_t Alignment = 8, template<size_t, size_t> class AllocPolicy = mem::SubAllocPolicy>
  class Arena; 

  template<size_t Cap, size_t Alignment>
  class Arena<Cap, Alignment, mem::MAllocPolicy> : private mem::MAllocPolicy<Cap, Alignment> {
  public:
    using alloc_t = mem::MAllocPolicy<Cap, Alignment>;
    Arena() : alloc_t(), top(0) {
      alloc_t::allocate();
    }
    ~Arena() {
      alloc_t::deallocate();
    }

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      if(new_top > Cap) {
        return nullptr;
      }
      result += top;
      top = new_top;
      return (void*)result;
    }

    void pop_to(size_t pos) {
      if(pos >= top) {
        return;
      }
      top = pos;
    }

    void pop_amount(size_t amount) {
      size_t new_top = top - amount;
      pop_to(new_top);
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
  
  private:
    size_t top = 0;
  };


  template<size_t Cap, size_t Alignment>
  class Arena<Cap, Alignment, mem::StackAllocPolicy> : private mem::StackAllocPolicy<Cap, Alignment> {
  public:
    using alloc_t = mem::StackAllocPolicy<Cap, Alignment>;
    Arena() : alloc_t(), top(0) {}
    ~Arena() {}

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      L_ASSERT(new_top <= Cap);
      result += top;
      top = new_top;
      return (void*)result;
    }

    void pop_to(size_t pos) {
      if(pos >= top) {
        return;
      }
      top = pos;
    }

    void pop_amount(size_t amount) {
      size_t new_top = top - amount;
      pop_to(new_top);
    }

    const void* get_buffer() {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
  
  private:
    size_t top = 0;
  };


  template<size_t Cap, size_t Alignment>
  class Arena<Cap, Alignment, mem::SubAllocPolicy> : private mem::SubAllocPolicy<Cap, Alignment> {
  public:
    using alloc_t = mem::SubAllocPolicy<Cap, Alignment>;
    using index_t = typename alloc_t::index_t;
    Arena(void* ptr) : alloc_t(ptr), top(0) {}

    void move_ptr(void* ptr) {
      alloc_t::move(ptr);
    }

    ~Arena() {}

    void* push(size_t size) {
      u8* result = (u8*)alloc_t::data();
      const size_t new_top = top + size;
      if(new_top > Cap) {
        return nullptr;
      }
      result += top;
      top = new_top;
      return (void*)result;
    }

    void pop_to(size_t pos) {
      if(pos >= top) {
        return;
      }
      top = pos;
    }

    void pop_amount(size_t amount) {
      size_t new_top = top - amount;
      pop_to(new_top);
    }

    // for reading only?
    const void* get_buffer() {
      return alloc_t::data();
    }

    void clear() {
      top = 0;
    }
  
  private:
    size_t top = 0;
  };

  namespace mem {
    using default_bucket_descriptor_set_t
          = List
            < defaults::Bucket_8<16384>
            , defaults::Bucket_64<16384>
            , defaults::Bucket_256<16384>
            , defaults::Bucket_1024<4096>
            , defaults::Bucket_16384<1024>
            , defaults::Bucket_256KB<128>
            , defaults::Bucket_1M<64>
            , defaults::Bucket_16M<16>>;

#if defined LOFI_DEFAULT_BUCKETS_COUNT
// if using custom buckets
#if (LOFI_DEFAULT_BUCKETS_COUNT == 1)

      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

#elif (LOFI_DEFAULT_BUCKETS_COUNT == 2)
      
      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
#elif (LOFI_DEFAULT_BUCKETS_COUNT == 3)
 
      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
      template<>
      struct BucketDescriptorSet<2> {
        using type = default_bucket_descriptor_set_t;
      };
 
#elif (LOFI_DEFAULT_BUCKETS_COUNT == 4)

      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
      template<>
      struct BucketDescriptorSet<2> {
        using type = default_bucket_descriptor_set_t;
      };
 
      template<>
      struct BucketDescriptorSet<3> {
        using type = default_bucket_descriptor_set_t;
      };
 
#else

      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

      template<>
      struct BucketDescriptorSet<1> {
        using type = default_bucket_descriptor_set_t;
      };
   
      template<>
      struct BucketDescriptorSet<2> {
        using type = default_bucket_descriptor_set_t;
      };
 
      template<>
      struct BucketDescriptorSet<3> {
        using type = default_bucket_descriptor_set_t;
      };
 
      template<>
      struct BucketDescriptorSet<4> {
        using type = default_bucket_descriptor_set_t;
      };

#endif
#else
      template<>
      struct BucketDescriptorSet<0> {
        using type = default_bucket_descriptor_set_t;
      };

#endif
  }		// -----  end of namespace mem  ----- 
}   // -----  end of namespace lofi  -----
