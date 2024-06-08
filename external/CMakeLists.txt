project(external C CXX)

# Relacy Race Detector (https://github.com/dvyukov/relacy)
set(RELACY_SRC_DIR "relacy/")
file(GLOB Relacy_SRC
     "${RELACY_SRC_DIR}/relacy/*.h"
     "${RELACY_SRC_DIR}/relacy/*.hpp"
)
add_library(relacy INTERFACE ${Relacy_SRC})

target_include_directories(relacy SYSTEM INTERFACE ${RELACY_SRC_DIR})
add_library(ext::relacy ALIAS relacy)
