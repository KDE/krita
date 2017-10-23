message(STATUS "> Retrieve G'MIC Standard Library...")
file(DOWNLOAD http://gmic.eu/gmic_stdlib.h ${CMAKE_CURRENT_BINARY_DIR}/src/gmic_stdlib.h)
message(STATUS " done!")

message(STATUS "> Retrieve CImg Library...")
file(DOWNLOAD https://github.com/dtschump/CImg/raw/master/CImg.h ${CMAKE_CURRENT_BINARY_DIR}/src/CImg.h)
message(STATUS " done!")
