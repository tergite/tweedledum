/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Qubit.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Operators/Standard/X.h"

#include <cassert>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum::tfc {
namespace {

inline void left_trim(std::string& str, char const* chars = " \t\n\v\f\r")
{
    if (str.empty()) {
        return;
    }
    std::size_t pos = str.find_first_not_of(chars);
    if (pos == std::string::npos) {
        str.clear();
    } else if (pos > 0) {
        str = str.substr(pos, std::string::npos);
    }
}

inline std::vector<std::string> split(std::string const& line)
{
    std::vector<std::string> slipt_string;

    std::istringstream iss(line);
    std::for_each(std::istream_iterator<std::string>(iss),
      std::istream_iterator<std::string>(), [&slipt_string](auto const& str) {
          std::size_t begin = 0;
          std::size_t end = str.find(',', begin);
          if (end == std::string::npos) {
              slipt_string.emplace_back(str);
              return;
          }
          while (end != std::string::npos) {
              std::string substr = str.substr(begin, end - begin);
              left_trim(substr);
              slipt_string.emplace_back(substr);
              begin = end + 1;
              end = str.find(',', begin);
          }
          if (begin < str.size()) {
              slipt_string.emplace_back(str.substr(begin));
          }
      });
    return slipt_string;
}

Circuit parse_stream(std::istream& buffer)
{
    Circuit circuit;
    std::unordered_map<std::string, Qubit> qubits;
    std::string line;

    while (std::getline(buffer, line)) {
        left_trim(line);
        if (line.empty() || line.at(0) == '#') {
            continue;
        }
        auto entries = split(line);
        if (line.at(0) == '.') {
            if (entries.at(0) == ".v") {
                std::for_each(entries.begin() + 1, entries.end(),
                [&](std::string const& label) {
                    qubits.emplace(label, circuit.create_qubit(label));
                });
            }
            continue;
        } else if (entries.at(0) == "BEGIN" || entries.at(0) == "END") {
            continue;
        }
        assert(entries.at(0).at(0) == 'f' || entries.at(0).at(0) == 't');
        Operator op = Op::X();
        if (entries.at(0).at(0) == 'f') {
            op = Op::Swap();
        }
        std::vector<Qubit> op_qubits;
        std::for_each(
          entries.begin() + 1, entries.end(), [&](std::string const& label) {
              Qubit qubit = Qubit::invalid();
              if (label.back() == '\'') {
                  qubit = !qubits.at(label.substr(0, label.size() - 1));
              } else {
                  qubit = qubits.at(label);
              }
              op_qubits.push_back(qubit);
          });
        circuit.apply_operator(op, op_qubits);
    }
    return circuit;
}

} // namespace

Circuit parse_source_buffer(std::string_view buffer)
{
    std::istringstream iss{std::string(buffer)};
    return parse_stream(iss);
}

Circuit parse_source_file(std::string_view path)
{
    std::ifstream stream{std::string(path)};
    assert(stream.is_open());
    Circuit circuit = parse_stream(stream);
    stream.close();
    return circuit;
}

} // namespace tweedledum::tfc
