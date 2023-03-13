if (Qt6_FOUND)
  find_package(Qt6 COMPONENTS Test)
  set(Qt Qt)
else()
  find_package(Qt5 COMPONENTS Test)
  set(Qt Qt5)
endif()

add_executable(test_QtQuickStream
  test_main.cpp
)

target_include_directories(test_QtQuickStream
  PRIVATE
    ../src
    ../include
    include
)

target_link_libraries(test_QtQuickStream
  PRIVATE
    QtQuickStream::QtQuickStream
    ${Qt}::Test
)

add_test(
  NAME test_QtQuickStream
  COMMAND
    $<TARGET_FILE:test_QtQuickStream>
    $<$<BOOL:${NE_FORCE_TEST_COLOR}>:--use-colour=yes>
)
