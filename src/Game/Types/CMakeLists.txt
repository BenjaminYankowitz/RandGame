add_library(GameTypes STATIC)  
target_sources(GameTypes
	PUBLIC
		FILE_SET CXX_MODULES FILES 
		GameTypes.cxx GameTime.cxx Misc.cxx)