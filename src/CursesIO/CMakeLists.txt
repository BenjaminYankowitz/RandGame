add_library(CursesLowLevel STATIC)
  
target_sources(CursesLowLevel
	PUBLIC
		FILE_SET CXX_MODULES FILES
			CursesLowLevel.cxx)
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
target_include_directories(CursesLowLevel PRIVATE ${CURSES_INCLUDE_DIR})
target_link_libraries(CursesLowLevel PRIVATE ${NCURSES_LIBRARIES})

add_library(Printing STATIC)
  
target_sources(Printing
	PUBLIC
		FILE_SET CXX_MODULES FILES
			Printing.cxx)
target_link_libraries(Printing PUBLIC CursesLowLevel Common GameInterface)

add_library(CursesIOHelper STATIC)

target_sources(CursesIOHelper
	PUBLIC
		FILE_SET CXX_MODULES FILES
			CursesIOHelper.cxx)

target_link_libraries(CursesIOHelper PUBLIC GameInterface CursesLowLevel Common Printing)

add_library(CursesIO STATIC)

target_sources(CursesIO
	PUBLIC
		FILE_SET CXX_MODULES FILES
			CursesIO.cxx
	PRIVATE
		CursesIOViewer.cxx
		CursesIOInterface.cxx
		CursesIOActions.cxx)

target_link_libraries(CursesIO PUBLIC GameInterface CursesLowLevel Common Printing CursesIOHelper)