#include "sysy_driver.hh"
#include "sysy_parser.hh"
#include <sstream>

sysy_driver::sysy_driver() : trace_scanning(false), trace_parsing(false) {
  variables["one"] = 1;
  variables["two"] = 2;
}

sysy_driver::~sysy_driver() {}

SyntaxCompUnit *sysy_driver::parse(const std::string &f) {
  file = f;
  lexer.set_debug(trace_scanning);

  // Try to open the file:
  instream.open(file);

  std::istringstream poss_stringstream(preprocess());

  lexer.switch_streams(&poss_stringstream, 0);

  yy::sysy_parser parser(*this);
  parser.set_debug_level(trace_parsing);
  parser.parse();
  scan_end();
  // std::cout << this->root;
  return this->root;
}

void sysy_driver::error(const yy::location &l, const std::string &m) {
  std::cerr << l << ": " << m << std::endl;
}

void sysy_driver::error(const std::string &m) { std::cerr << m << std::endl; }

void sysy_driver::scan_begin() {
  lexer.set_debug(trace_scanning);

  // Try to open the file:
  instream.open(file);

  std::istringstream poss_stringstream(preprocess());
  // poss_stream << poss_stringstream;

  // std::string line;
  // while (getline(poss_stream, line, '\n'))
  // {
  //     std::cout<<line<<"\n";
  // }

  lexer.switch_streams(&poss_stringstream, 0);
}

void sysy_driver::scan_end() { instream.close(); }

std::string sysy_driver::preprocess() {
  std::string poss;

  int line_pos = 0;
  std::string line;
  std::string word;

  while (getline(instream, line, '\n')) {
    line_pos++;
    // std::cout << line<< std::endl;
    std::stringstream s;
    s << line;

    while (getline(s, word, ' ')) {
      int tab_num;
      tab_num = word.find_first_not_of("\t");
      word.erase(0, tab_num);
      //
      for (int i = 0; i < tab_num; i++) {
        poss += "\t";
      }

      if (word == "starttime();") {
        poss += "_sysy_starttime(";
        poss += std::to_string(line_pos);
        poss += "); ";
      } else if (word == "stoptime();") {
        poss += "_sysy_stoptime(";
        poss += std::to_string(line_pos);
        poss += "); ";
      } else {
        poss += word;
        poss += " ";
      }
    }
    poss += "\n";
  }
  // std::cerr<<poss<<std::endl;
  return poss;
}