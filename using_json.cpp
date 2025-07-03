#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <fstream>
#include "Header.h"

// Alias for convenience
//using json = nlohmann::json;

// Define a complex structure
struct Address {
  std::string street;
  std::string city;
  std::optional<std::string> state;
  int postal_code;

  // Convert Address struct to json
  friend void to_json(nlohmann::json& j, const Address& address) {
    j = nlohmann::json{
        {"street", address.street},
        {"city", address.city},
        {"state", address.state.value_or("N/A")}, // Use "N/A" if no state is provided
        {"postal_code", address.postal_code}
    };
  }
};

struct Employee {
  std::string name;
  int age;
  std::vector<Address> addresses;
  std::map<std::string, std::optional<std::string>> skills; // Key-value pair with optional values
  bool is_manager;

  // Convert Employee struct to json
  friend void to_json(nlohmann::json& j, const Employee& emp) {
    // Create a new json object for skills, manually converting optionals
    nlohmann::json skills_json = nlohmann::json::object();
    for (const auto& [skill, level] : emp.skills) {
      // If level has a value, use it; otherwise use nullptr to represent null
      skills_json[skill] = level.has_value() ? nlohmann::json(level.value()) : nlohmann::json(nullptr);
    }

    j = nlohmann::json{
        {"name", emp.name},
        {"age", emp.age},
        {"addresses", emp.addresses},
        {"skills", skills_json},  // Insert the converted skills json
        {"is_manager", emp.is_manager}
    };
  }
};

// Function that creates and returns a complex JSON structure
nlohmann::json create_company_structure() {
  // Create some addresses
  Address addr1{ "123 Maple St", "Springfield", std::nullopt, 12345 };
  Address addr2{ "456 Oak Ave", "Shelbyville", "IL", 54321 };

  // Create an employee with nested data
  Employee emp{
      "John Doe",
      30,
      {addr1, addr2}, // List of addresses
      {{"C++", "Expert"}, {"Python", std::nullopt}, {"Java", "Intermediate"}}, // Some skills, some unknown
      true // Is a manager
  };

  // Create a company structure with multiple employees
  nlohmann::json company = {
      {"company_name", "Tech Solutions"},
      {"employees", {
          emp, // Add the first employee
          {
              {"name", "Jane Smith"},
              {"age", 28},
              {"addresses", {
                  {{"street", "789 Pine Rd"}, {"city", "Capital City"}, {"state", "CA"}, {"postal_code", 67890}}
              }},
              {"skills", {{"JavaScript", "Intermediate"}, {"HTML", "Advanced"}}},
              {"is_manager", false}
          }
      }},
      {"departments", {"Engineering", "Marketing", "HR"}}, // Array of departments
      {"founded_year", 1999},
      {"is_public", true}
  };

  return company;
}

int json_sample() {
  // Create the complex structure and serialize it to JSON
  nlohmann::json company_json = create_company_structure();

  // Pretty-print the JSON structure
  std::cout << company_json.dump(2) << std::endl;

  return 0;
}


void ProcessJson(const ControlLevel& desktop)
{
  nlohmann::ordered_json ordered_desktop(desktop);
  std::ofstream outFile("pretty.json");
  outFile << std::setw(2) << ordered_desktop.dump(1, ' ', false, nlohmann::ordered_json::error_handler_t::replace) << std::endl;

  nlohmann::json efficient_desktop(desktop);
  std::ofstream outFile2("not_pretty.json");
  outFile2 << std::setw(2) << efficient_desktop.dump(1, ' ', false, nlohmann::ordered_json::error_handler_t::replace) << std::endl;

  std::ifstream f("pretty.json");
  nlohmann::json efficient_desktop_back = nlohmann::json::parse(f);
  ControlLevel back = efficient_desktop_back.template get<ControlLevel>();
  return;
}