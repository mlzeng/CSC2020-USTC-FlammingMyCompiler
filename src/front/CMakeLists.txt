#add_executable(
#        SYSYC_AST
#        sysy.cc
#)

add_library(
        driver
        sysy_driver.cc
)

add_library(
        scanner STATIC
        sysy_scanner.cc
)

add_library(
        parser STATIC
        sysy_parser.cc
)

#target_compile_options(
#        parser
#        PRIVATE "-frtti"
#)
#target_link_libraries(
#        SYSYC_AST
#        driver
#        parser
#        scanner
#        syntax_tree
#)

target_link_libraries(
        driver
        parser
        scanner
        syntax_tree
)