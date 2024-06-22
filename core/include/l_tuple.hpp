// =====================================================================================
//
//       Filename:  l_tuple.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-09 4:08:50 PM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_vocab.hpp"

// thank you to Odin Holmes and Chiel Douwes


namespace lofi {

  template<typename I, typename T>
  struct tuple_element {
    static constexpr u64 index = I::value;
    using type = T;
    T elem;
  };

 
  template<typename... Ts>
  struct tuple_base : Ts... {
    using type = List<Ts...>;
  };

//  template<typename T, typename... Ts>
//  struct tuple_zip<Int<0>, T, Ts...> : tuple_zip<Int<1>, Ts>... {
//    using type = List<tuple_element<0, T>, typename tuple_zip<Int<1>, Ts...>::type>; 
//  };

  template<typename... Ts>
  using tuple_zip = typename zipper<tuple_element, typename IdxSequence<sizeof...(Ts)>::type, Ts...>::type;

  template<typename... Ts>
  using make_tuple = typename meta::lift<tuple_base, tuple_zip<Ts...>>::type;

  template<typename... Ts>
  struct tuple : make_tuple<Ts...> {
    using base_t = make_tuple<Ts...>;
    using list_t = typename base_t::type;

    template<u64 Index>
    auto& get() {
      return static_cast<meta::at_t<list_t, Index>&>(*this).elem;
    }
  };

  namespace meta {
      template<u64 Index, typename... Ts>
      auto& get(tuple<Ts...>& t) {
        return static_cast<meta::at_t<typename tuple<Ts...>::type, Index>&>(t);
      };
  }		// -----  end of namespace meta  ----- 

}		// -----  end of namespace lofi  ----- 
