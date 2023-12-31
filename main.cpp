#include <fstream>
#include <iostream>
#include <string_view>

#include "json.h"
#include "json_reader.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

       json::Document doc = json::Load(std::cin);
       JSONReader reader(doc.GetRoot());

       reader.LoadData();
       reader.Serialization();

    } else if (mode == "process_requests"sv) {

       json::Document doc = json::Load(std::cin);
       JSONReader reader(doc.GetRoot());

       reader.Deserialization();
       reader.ProcessQuery(std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
