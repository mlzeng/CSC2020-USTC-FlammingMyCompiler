#include <iostream>
#include "sysy_driver.hh"
// #include "syntax_tree_printer.hh"

sysy_driver driver;
auto printer=new syntax_tree_printer;
SyntaxCompUnit *root;

int main (int argc, char *argv[])
{
    std::cout << "start" << std::endl;
    int res = 0;
    for (int i = 1; i < argc; ++i)
        if (argv[i] == std::string ("-p"))
            driver.trace_parsing = true;
        else if (argv[i] == std::string ("-s"))
            driver.trace_scanning = true;
        else if ( root = driver.parse(argv[i]) ){
            std::cout << driver.root << "KO";
            printer->visit(*root);
        }
        else
            res = 1;
    return res;
}
