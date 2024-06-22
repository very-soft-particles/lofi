// =====================================================================================
//
//       Filename:  l_meta.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-03 9:57:11 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_base.hpp"

namespace lofi {
  namespace meta {
 
    struct identity {
      template<typename T>
      using f = T;
    };
  
    struct make_list {
      template<typename... Ts>
      using f = List<Ts...>;
    };


    static constexpr u64 index_switch_sizes[] = {256, 65536, 4294967296};
    static constexpr u64 index_max_sizes[] = {MAX_u8, MAX_u16, MAX_u32, MAX_u64};

    template<size_t Size>
    static constexpr u8 get_type_table_index 
            = Size <= index_switch_sizes[0] ? 
                0 : 
                Size <= index_switch_sizes[1] ? 
                  1 :
                  Size <= index_switch_sizes[2] ?
                    2 : 3;

    template<u8 index>
    struct index_type_table;

    template<>
    struct index_type_table<0> {
      using type = u8;
    };

    template<>
    struct index_type_table<1> {
      using type = u16;
    };

    template<>
    struct index_type_table<2> {
      using type = u32;
    };

    template<>
    struct index_type_table<3> {
      using type = u64;
    };

    template<size_t Size>
    using choose_index_type_t = typename meta::index_type_table<meta::get_type_table_index<Size>>::type;

  } // end of namespace meta
//
//  template<template<class> class Filter, class TupleIn, class TupleOut = std::tuple<>>
//  struct filter;
//
//  template<template<class> class Filter, class TypeIn, class... TypesIn, class... TypesOut>
//  struct filter<Filter, std::tuple<TypeIn, TypesIn...>, std::tuple<TypesOut...>> {
//    using type 
//      = std::conditional_t
//      < Filter<TypeIn>::value
//      , typename filter
//        < Filter
//        , std::tuple<TypesIn...>
//        , std::tuple<TypeIn, TypesOut...>
//        >::type
//      , typename filter
//        < Filter
//        , std::tuple<TypesIn...>
//        , std::tuple<TypesOut...>
//        >::type
//      >;
//  };
//
//  template<template<class> class Filter, class TypeIn, class... TypesIn>
//  struct filter<Filter, std::tuple<TypeIn, TypesIn...>, std::tuple<>> {
//    using type 
//      = std::conditional_t
//      < Filter<TypeIn>::value
//      , typename filter
//        < Filter
//        , std::tuple<TypesIn...>
//        , std::tuple<TypeIn>
//        >::type
//      , typename filter
//        < Filter
//        , std::tuple<TypesIn...>
//        , std::tuple<>
//        >::type
//      >;
//  };
//
//  template<template<class> class Filter, class... TypesOut>
//  struct filter<Filter, std::tuple<>, std::tuple<TypesOut...>> {
//    using type = std::tuple<TypesOut...>;
//  };
//
//  template<template<class> class Filter>
//  struct filter<Filter, std::tuple<>, std::tuple<>> {
//    using type = std::tuple<>;
//  };
//
//  template<class Tuple>
//  struct strip_first;
//
//  template<class Type, class... Types>
//  struct strip_first<std::tuple<Type, Types...>> {
//    using type = Type;
//    using cdr = std::tuple<Types...>;
//  };
//
//  template<class Type>
//  struct strip_first<std::tuple<Type>> {
//    using type = Type;
//  };
//
//  template<class T>
//  struct wrapper_t {
//    using type = T;
//  };
//
//  template <class Tup, class Func, std::size_t ...Is>
//  constexpr void static_for_impl(Tup&& t, Func &&f, std::index_sequence<Is...> )
//  {
//      ( ..., f(std::integral_constant<std::size_t, Is>{}, std::get<Is>(t)) );
//  }
//  
//  template <class ... T, class Func >
//  constexpr void static_for(std::tuple<T...>&&t, Func &&f)
//  {
//      static_for_impl(t, std::forward<Func>(f), std::make_index_sequence<sizeof...(T)>{});
//  }
//
//  template<class Tuple, size_t... Is>
//  constexpr void for_types_impl(auto&& f, std::index_sequence<Is...>) {
//    ( f(wrapper_t<std::tuple_element_t<Is, Tuple>>{}), ...);
//  }
//
//  template<class Tuple>
//  constexpr void for_types(auto&& f)
//  {
//    for_types_impl<Tuple>(f, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
//  }
//


}		// -----  end of namespace lofi  ----- 
