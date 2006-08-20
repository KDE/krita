########### next target ###############

set(krosstest_SRCS 
   testobject.cpp 
   testwindow.cpp 
   main.cpp )

kde4_automoc(${krosstest_SRCS})

kde4_add_executable(krosstest ${krosstest_SRCS})

target_link_libraries(krosstest  ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} krosscore )

