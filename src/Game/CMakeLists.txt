add_subdirectory(Types)
add_library(DungeonMaker STATIC)
target_sources(DungeonMaker
	PUBLIC
		FILE_SET CXX_MODULES FILES
        DungeonMaker.cxx)
target_link_libraries(DungeonMaker PUBLIC Common PerlinNoise OpenSimplex)

add_library(MonsterClassConfig STATIC)  
target_sources(MonsterClassConfig
	PUBLIC
		FILE_SET CXX_MODULES FILES 
		MonsterClassConfig.cxx)

target_link_libraries(MonsterClassConfig PUBLIC Common GameTypes)

add_library(Game STATIC)  
target_sources(Game
	PUBLIC
		FILE_SET CXX_MODULES FILES 
      	Game.cxx)
            
target_link_libraries(Game PUBLIC DungeonMaker Common MonsterClassConfig GameTypes)

add_library(GameInterface STATIC)
target_sources(GameInterface
	PUBLIC
		FILE_SET CXX_MODULES FILES
      	GameInterface.cxx
	PRIVATE
				GameInterfaceImpl.cxx)

target_link_libraries(GameInterface PUBLIC Common GameTypes PRIVATE Game)