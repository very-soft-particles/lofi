
set(LOFI_INTERNAL_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/include")

add_subdirectory(include)

set(LOFI_HEADERS
  "${LOFI_INCLUDE_PATH}/l_base.hpp"
  "${LOFI_INCLUDE_PATH}/l_memory.hpp"
  "${LOFI_INCLUDE_PATH}/l_meta.hpp"
  "${LOFI_INCLUDE_PATH}/l_string.hpp"
  "${LOFI_INCLUDE_PATH}/l_sync.hpp"
  "${LOFI_INCLUDE_PATH}/l_thread.hpp"
  "${LOFI_INCLUDE_PATH}/l_tuple.hpp"
  "${LOFI_INCLUDE_PATH}/l_variant.hpp"
  "${LOFI_INCLUDE_PATH}/l_vocab.hpp"
  "${LOFI_INCLUDE_PATH}/l_fiber.hpp"

  # using potentially optimized compile time dispatch for faster compiling
  # ${LOFI_META_VOCAB}
)


