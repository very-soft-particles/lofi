// =====================================================================================
//
//       Filename:  l_vocab.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-11 11:20:12 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once

#if(OS_WINDOWS)
#include <Windows.h>
#endif
#include "l_meta.hpp"
#include "inline/preprocessor.hpp"

#ifdef LOFI_USE_META_VOCAB
  #include "meta/vocab.hpp"
#else

namespace lofi {
    namespace meta {

      template<size_t I, class TFind, class... Ts>
        struct find : IdxT<MAX_u64> {};

      template<size_t I, class TFind, class T, class... Ts>
        struct find<I, TFind, T, Ts...> : find<I + 1, TFind, Ts...> {};

      template<size_t I, class TFind, class... Ts>
        struct find<I, TFind, TFind, Ts...> : IdxT<I> {};

      template<class TFind>
      struct find_t {
        template<typename... Ts>
        using type = typename find<0, TFind, Ts...>::type;

      };

      template<size_t I, bool Found, template<typename...> class Pred, typename... Ts>
        struct pred_find : IdxT<MAX_u64> {};

      template<size_t I, template<typename...> class Pred, typename... Ts>
        struct pred_find<I, true, Pred, Ts...> : IdxT<I - 1> {};                  // changed to I - 1
                                                                                // because the bucket
                                                                                // finder in the static
                                                                                // allocator was finding
                                                                                // one too high every time

      template<size_t I, template<typename...> class Pred, typename T, typename... Ts>
        struct pred_find<I, false, Pred, T, Ts...> : pred_find<I + 1, Pred<T>::value, Pred, Ts...> {};

      template<template<typename...> class TPred>
      struct pred_find_t {
        template<typename... Ts>
        using type = typename pred_find<0, false, TPred, Ts...>::type;
      };

      template<typename T>
      struct less_t {
        template<typename U>
        using type = Bool<(T::value <= U::value)>;
      };



      template<typename Acc = Int<0>, typename... Is>
        struct sum : Acc {};

      template<typename Acc, typename I, typename... Is>
        struct sum<Acc, I, Is...> : sum<Int<Acc::value + I::value>, Is...> {};

      template<typename... Is>
      using apply_sum = sum<Int<0>, Is...>;
      template<typename TOut, typename TDelim, typename... Ts>
        struct join {
          static_assert(AlwaysFalse<TOut>::value, "incorrect parameter format in join, expected a list of lists");
        };

      template<typename... Os, typename TDelim, typename... Ls, typename... Ts>
        struct join<List<Os...>, TDelim, List<Ls...>, Ts...> : join<List<Os...,TDelim,Ls...>,TDelim, Ts...> {};

      template<typename... Os, typename TDelim>
        struct join<List<Os...>, TDelim> : Return<List<Os...>> {};

      template<typename TOut, typename TCur, typename TDelim, typename... Ts>
        struct split;

      template<typename... Os, typename... Cs, typename TDelim, typename T, typename... Ts> // next is not delim 
        struct split<List<Os...>, List<Cs...>, TDelim, T, Ts...>                              // more left
        : split<List<Os...>, List<Cs..., T>, TDelim, Ts...> {};                               

      template<typename... Os, typename... Cs, typename TDelim, typename... Ts>             // next is delim
        struct split<List<Os...>, List<Cs...>, TDelim, TDelim, Ts...>                         // more left
        : split<List<Os..., List<Cs...>>, List<>, TDelim, Ts...> {};                           

      template<typename... Os, typename... Cs, typename TDelim, typename T>                 // next is not delim
        struct split<List<Os...>, List<Cs...>, TDelim, T> {                                   // no more left
          using type = List<Os..., List<Cs...,T>>;
        };                           

      template<typename... Os, typename... Cs, typename TDelim>                             // next is delim
        struct split<List<Os...>, List<Cs...>, TDelim, TDelim> {                              // no more left
          using type = List<Os..., List<Cs...>>;
        };                           

      // same but with empty current list
      template<typename... Os, typename TDelim, typename... Ts>                             // next is delim
        struct split<List<Os...>, List<>, TDelim, TDelim, Ts...>                              // more left
        : split<List<Os...>, List<>, TDelim, Ts...> {};                           

      template<typename... Os, typename TDelim>                                             // next is delim
        struct split<List<Os...>, List<>, TDelim, TDelim> {                                   // no more left
          using type = List<Os...>;
        };

      template<typename TList, typename TIndex>
      struct at {
        static_assert(AlwaysFalse<TList>::value, "implausible type");
      };

      template<typename T, typename...Ts, size_t I>
      struct at<List<T, Ts...>, IdxT<I>> : at<List<Ts...>, IdxT<I-1>> {};

      template<typename T, typename ...Ts>
      struct at<List<T, Ts...>, IdxT<0>> : Return<T> {};

      template<typename TList, size_t Index>
      using at_t = typename at<TList, IdxT<Index>>::type;

      template<typename TOut, typename... Ts>
      struct append;
    
      template<typename... Os, typename... Ts>
      struct append<List<Os...>, List<Ts...>> : Return<List<Os..., Ts...>> {};

      template<template<typename...> typename F, typename TList>
      struct lift;

      template<template<typename...> typename F, typename... Ts>
      struct lift<F, List<Ts...>> {
        using type = F<Ts...>;
      };

      template<class T, template<class...> class ListT, size_t N>
      struct repeat 
      : append<
          typename repeat<T, ListT, N/2>::type,
          typename repeat<T, ListT, N - N/2*2>::type
        > {};
      
      template<class T, template<class...> class ListT>
      struct repeat<T, ListT, 1> {
        using type = ListT<T>;
      };
    
      template<class T, template<class...> class ListT>
      struct repeat<T, ListT, 0> {
        using type = ListT<>;
      };
    
      template<template<class...> class T>
      struct template_t {
        template<typename... Ts>
        using type = T<Ts...>;
      };
    
      template<template<class...>class T, typename... Ts>
      using apply = typename template_t<T>::template type<Ts...>::type;
    
    
      template<typename T, typename U>
      struct pair {
        using type = pair<T, U>;
        using first = T;
        using second = U;
      };

      template
        < typename TOut
        , template<typename, typename> class TPred
        , typename TInsert, bool BTag, typename... Ts>
        struct sort_insert;

      template
        < typename... Os
        , template<typename, typename> class TPred
        , typename TInsert, typename T1, typename T2, typename... Ts>
        struct sort_insert<List<Os...>, TPred, TInsert, true, T1, T2, Ts...>
        : sort_insert<List<Os..., T1>, TPred, TInsert, TPred<T2, TInsert>::value, T2, Ts...> {};

      template
        < typename... Os
        , template<typename, typename> class TPred
        , typename TInsert, typename... Ts>
        struct sort_insert<List<Os...>, TPred, TInsert, true, Ts...> {
          using type = List<Os..., Ts..., TInsert>;
        };

      template
        < typename... Os
        , template<typename, typename> class TPred
        , typename TInsert, typename... Ts>
        struct sort_insert<List<Os...>, TPred, TInsert, false, Ts...> {
          using type = List<Os..., TInsert, Ts...>;
        };

    }		// -----  end of namespace meta  ----- 

  template<typename TOut, template<typename, typename> class P, typename... Ts>
    struct sort {
      static_assert(AlwaysFalse<TOut>::value, "implausible parameters");
    };

  template<typename O, typename... Os, template<typename, typename> class TPred, typename TInsert, typename... Ts>
    struct sort<List<O, Os...>, TPred, TInsert, Ts...> : sort<typename meta::sort_insert<List<>, TPred, TInsert, TPred<O, TInsert>::value, O, Os...>::type, TPred, Ts...> {};

  template<typename... Os, template<typename, typename> class TPred, typename TInsert, typename... Ts>
    struct sort<List<Os...>, TPred, TInsert, Ts...> : sort<typename meta::sort_insert<List<>, TPred, TInsert, false, Os...>::type, TPred, Ts...> {};

  template<typename... Os, template<typename, typename> class TPred, typename... Ts>
    struct sort<List<Os...>, TPred, Ts...>  {
      using type = List<Os...>;
    };

  template<typename TemplateT>
  struct template_size;

  template<template<typename...> class TemplateT, typename... Ts>
  struct template_size<TemplateT<Ts...>> : Value<size_t, sizeof...(Ts)> {};
  
  template<typename ListT>
  using list_size = template_size<ListT>; 

  template<typename T, typename U>
    struct is_same {
      using type = FalseType;
    };

  template<typename T>
    struct is_same<T, T> {
      using type = TrueType;
    };

  template<class T, class ListT>
    struct get_list_index;

  template<class T, class... Types>
    struct get_list_index<T, List<T, Types...>> {
      static constexpr size_t value = 0;
    };

  template<class T, class U, class... Types>
    struct get_list_index<T, List<U, Types...>> {
      static constexpr size_t value = 1 + get_list_index<T, List<Types...>>::value;
    };

  template<class T, class Tuple>
    struct has_type;

  template<class T, class... Types>
    struct has_type<T, List<T, Types...>> {
      static constexpr bool value = true;
    };

  template<class T, class U, class... Types>
    struct has_type<T, List<U, Types...>> {
      static constexpr bool value = has_type<T, List<Types...>>::value;
    };

  template<class T>
    struct has_type<T, List<>> {
      static constexpr bool value = false;
    };

  template<int N, typename... Is>
    struct IntSequence : IntSequence<N-1, Int<N-1>, Is...> {};

  template<typename... Is>
    struct IntSequence<0, Is...> {
      using type = List<Is...>;
    };

  template<size_t N, typename... Is>
    struct IdxSequence : IdxSequence<N-1, IdxT<N-1>, Is...> {};

  template<typename... Is>
    struct IdxSequence<0, Is...> {
      using type = List<Is...>;
    };


  struct identity {
    template<typename T>
      using f = T;
  };

  struct make_list {
    template<typename... Ts>
      using f = List<Ts...>;
  };

  template<typename Is>
  static constexpr size_t index_array[] = {MAX_u64};

  template<size_t... Is>
  static constexpr size_t index_array<List<IdxT<Is>...>>[] = {Is...};

  template<template<typename, typename> class H, typename TList, typename... Ts>
  struct zipper;

  template<template<typename, typename> class H, typename... Is, typename... Ts>
  struct zipper<H, List<Is...>, Ts...> {
    using type = List<H<Is, Ts>...>;
  };
  
  template<size_t Size, typename Op>
  struct dispatcher {
    using index_t = IdxT<Size>;

    constexpr dispatcher(IdxT<Size>, Op op) : _op(op) {}

    auto operator()(size_t i) const {
      return impl<0>(i);
    }
  private:
    Op _op;

    template<size_t Start>
    constexpr auto impl(size_t i) const {
#define CASE(x) case Start + x: if constexpr (Size > Start + x) return _op(IdxV<Start + x>);
      switch(i) {
        L_INDICES_TO_512(CASE)
        default: if constexpr (Size >= Start + 512) return impl<Start + 512>(i);
                 else L_ASSERT(false);
      }
    }
  };

}		// -----  end of namespace lofi  ----- 

#endif
