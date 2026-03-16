include(CTest)
find_package(GTest CONFIG REQUIRED)

add_library(TestHeader INTERFACE TestHeader.h)
target_link_libraries(TestHeader INTERFACE GTest::gtest_main)

add_executable(TestPath TestPath.cpp)
target_link_libraries(TestPath PRIVATE Common TestHeader)
add_test(NAME TestPath COMMAND TestPath)

add_executable(PositionAndDirTests PositionAndDirTests.cpp)
target_link_libraries(PositionAndDirTests PRIVATE Common TestHeader)
add_test(NAME PositionAndDirTests COMMAND PositionAndDirTests)

add_executable(Static2DArrTests Static2DArrTests.cpp)
target_link_libraries(Static2DArrTests PRIVATE Common TestHeader)
add_test(NAME Static2DArrTests COMMAND Static2DArrTests)

add_executable(CompileTimeHashMapTests CompileTimeHashMapTests.cpp)
target_link_libraries(CompileTimeHashMapTests PRIVATE Common TestHeader)
add_test(NAME CompileTimeHashMapTests COMMAND CompileTimeHashMapTests)

add_executable(ObjectContainerTests ObjectContainerTests.cpp)
target_link_libraries(ObjectContainerTests PRIVATE Game TestHeader)
add_test(NAME ObjectContainerTests COMMAND ObjectContainerTests)

add_executable(DiceTests DiceTests.cpp)
target_link_libraries(DiceTests PRIVATE Common TestHeader)
add_test(NAME DiceTests COMMAND DiceTests)

add_executable(SerializationLibTests SerializationLibTests.cpp)
target_link_libraries(SerializationLibTests PRIVATE SerializationLib TestHeader)
add_test(NAME SerializationLibTests COMMAND SerializationLibTests)

add_executable(MiscTests MiscTests.cpp)
target_link_libraries(MiscTests PRIVATE Common TestHeader)
add_test(NAME MiscTests COMMAND MiscTests)

add_executable(GameTimeTests GameTimeTests.cpp)
target_link_libraries(GameTimeTests PRIVATE GameTypes TestHeader)
add_test(NAME GameTimeTests COMMAND GameTimeTests)

add_executable(MoveModeTests MoveModeTests.cpp)
target_link_libraries(MoveModeTests PRIVATE GameTypes TestHeader)
add_test(NAME MoveModeTests COMMAND MoveModeTests)

add_executable(MonsterConfigTests MonsterConfigTests.cpp)
target_link_libraries(MonsterConfigTests PRIVATE MonsterClassConfig TestHeader)
add_test(NAME MonsterConfigTests COMMAND MonsterConfigTests)

add_executable(MonsterIdTests MonsterIdTests.cpp)
target_link_libraries(MonsterIdTests PRIVATE Game TestHeader)
add_test(NAME MonsterIdTests COMMAND MonsterIdTests)

add_executable(PathIterableTests PathIterableTests.cpp)
target_link_libraries(PathIterableTests PRIVATE Common TestHeader)
add_test(NAME PathIterableTests COMMAND PathIterableTests)

add_executable(RandomTests RandomTests.cpp)
target_link_libraries(RandomTests PRIVATE Common TestHeader)
add_test(NAME RandomTests COMMAND RandomTests)

add_executable(GameInterfaceTests GameInterfaceTests.cpp)
target_link_libraries(GameInterfaceTests PRIVATE GameInterface TestHeader)
add_test(NAME GameInterfaceTests COMMAND GameInterfaceTests)
