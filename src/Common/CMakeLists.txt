add_library(SerialzationLib STATIC)
  
target_sources(SerialzationLib
	PUBLIC
		FILE_SET CXX_MODULES FILES
        SerialzationLib.cxx)

add_library(PerlinNoise STATIC)
  
target_sources(PerlinNoise
	PUBLIC
		FILE_SET CXX_MODULES FILES
        PerlinNoise.cxx)

target_link_libraries(PerlinNoise PUBLIC Common)

add_library(Common STATIC)
target_sources(Common
	PUBLIC
		FILE_SET CXX_MODULES FILES
            Common.cxx Shared.cxx Random.cxx CompileTimeHashMap.cxx Dice.cxx Misc.cxx IteratorBase.cxx Static2DArr.cxx SmallVector.cxx Location.cxx PathFind.cxx)