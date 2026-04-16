add_subdirectory(Types)
add_subdirectory(GameState)
add_library(DungeonMaker STATIC)
target_sources(DungeonMaker
	PUBLIC
		FILE_SET CXX_MODULES FILES
        DungeonMaker.cxx)
target_link_libraries(DungeonMaker PUBLIC Common PerlinNoise OpenSimplex GameTypes)

add_library(MonsterClassConfig STATIC)  
target_sources(MonsterClassConfig
	PUBLIC
		FILE_SET CXX_MODULES FILES 
		MonsterClassConfig.cxx)

target_link_libraries(MonsterClassConfig PUBLIC Common GameTypes)

add_library(GameInterface STATIC)
target_sources(GameInterface
	PUBLIC
		FILE_SET CXX_MODULES FILES
      	GameInterface.cxx
	PRIVATE
				GameInterfaceImpl.cxx)

target_link_libraries(GameInterface PUBLIC Common GameTypes PRIVATE GameState SerializationLib)