// Copyright 2020 Your Name <your_email>
#include "suggester_client.hpp"

const char suggestions_str[] = "suggestions";
const char input_str[] = "input";
const char text_str[] = "text";


std::string suggester_client::request() {
  nlohmann::json req;
  std::cout << "Enter a message: ";
  std::string input;
  std::cin >> input;
  req[input_str] = input;
  return req.dump();
}


void suggester_client::parse_suggest(const std::string& response_json,
                                     std::ostream& out) {
  nlohmann::json res;
  try {
    res = nlohmann::json::parse(response_json);
  } catch (const nlohmann::detail::parse_error& e) {
    throw std::runtime_error("Not json response");
  }
  if (!res[suggestions_str].empty()){
    out << " Maybe you wanted to type: " << std::endl;
    size_t count = 1;
    for (const auto& elem : res[suggestions_str]){
      out <<" " << count << ")" << std::setw(4)  << elem[text_str] << std::endl;
      ++count;
    }
  } else {
    out << " No suggestions for this input" << std::endl;
  }
}
