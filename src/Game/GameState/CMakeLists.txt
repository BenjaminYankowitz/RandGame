add_library(GameState STATIC)  
target_sources(GameState
	PUBLIC
		FILE_SET CXX_MODULES FILES
      	GameState.cxx
	PRIVATE
		GameSerialization.cxx GameStateImpl.cxx MonsterImpl.cxx) 
            
target_link_libraries(GameState PUBLIC DungeonMaker Common MonsterClassConfig GameTypes PRIVATE SerializationLib)