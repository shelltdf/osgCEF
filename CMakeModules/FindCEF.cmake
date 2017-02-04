
#  CEF_FOUND
#  CEF_INCLUDE_DIR
#  CEF_LIBRARY
#  CEF_LIBRARY_DEBUG
#  CEF_WRAPPER_LIBRARY
#  CEF_WRAPPER_LIBRARY_DEBUG

# $DEF_DIR is an environment variable that would
# correspond to the ./configure --prefix=$DEF_DIR

FIND_PATH( CEF_DIR include/cef_app.h )
            
FIND_PATH( CEF_INCLUDE_DIR cef_app.h
            ${CEF_DIR}/include
		  ) 

		 
FIND_LIBRARY(CEF_LIBRARY NAMES libcef PATHS ${CEF_DIR}/Release)
FIND_LIBRARY(CEF_LIBRARY_DEBUG NAMES libcef PATHS ${CEF_DIR}/Debug)

FIND_LIBRARY(CEF_WRAPPER_LIBRARY NAMES libcef_dll_wrapper 
            PATHS ${CEF_DIR}/libcef_dll/Release)
FIND_LIBRARY(CEF_WRAPPER_LIBRARY_DEBUG NAMES libcef_dll_wrapper 
            PATHS ${CEF_DIR}/libcef_dll/Debug)


SET( CEF_FOUND "NO" )
IF( CEF_INCLUDE_DIR)
    SET( CEF_FOUND "YES" )
ENDIF( CEF_INCLUDE_DIR )

