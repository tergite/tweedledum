/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_set.hpp"
#include "../../networks/qubit.hpp"
#include "../generic/rewrite.hpp"

#include <iostream>

namespace tweedledum {

/*! \brief Relative Phase Toffoli mapping
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `num_controls`
 * - `op`
 *
 * **Required network functions:**
 * - `add_gate`
 * - `foreach_cqubit`
 * - `foreach_cgate`
 * - `rewire`
 * - `rewire_map`
 */
template<typename Network>
Network rpt(Network const& src)
{
	auto gate_rewriter = [](auto& dest, auto const& gate) {
		if (gate.op().is(gate_set::mcx)) {
			switch (gate.num_controls()) {
			default:
				return false;

			case 0u: {
				gate.foreach_target([&](auto target) {
					dest.add_gate(gate_set::pauli_x, target);
				});
			} break;

			case 1u: {
				gate.foreach_control([&](auto control) {
					gate.foreach_target([&](auto target) {
						dest.add_gate(gate_set::cx, control, target);
					});
				});
			} break;

			case 2u: {
				uint32_t controls[2];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_set::cx, targets[0], targets[i]);
				}
				dest.add_gate(gate_set::hadamard, targets[0]);

				dest.add_gate(gate_set::cx, controls[1], targets[0]);
				dest.add_gate(gate_set::t_dagger, targets[0]);
				dest.add_gate(gate_set::cx, controls[0], targets[0]);
				dest.add_gate(gate_set::t, targets[0]);
				dest.add_gate(gate_set::cx, controls[1], targets[0]);
				dest.add_gate(gate_set::t_dagger, targets[0]);
				dest.add_gate(gate_set::cx, controls[0], targets[0]);
				dest.add_gate(gate_set::t, targets[0]);
				dest.add_gate(gate_set::cx, controls[0], controls[1]);
				dest.add_gate(gate_set::t_dagger, controls[1]);
				dest.add_gate(gate_set::cx, controls[0], controls[1]);
				dest.add_gate(gate_set::t, controls[1]);
				dest.add_gate(gate_set::t, controls[0]);

				dest.add_gate(gate_set::hadamard, targets[0]);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_set::cx, targets[0], targets[i]);
				}
			} break;

			case 3u: {
				uint32_t controls[3];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				const auto a = controls[0];
				const auto b = controls[1];
				const auto c = controls[2];
				const auto d = targets[0];
				auto helper = qid_invalid;
				for (auto i = 0u; i < dest.num_qubits(); ++i) {
					if (i != a && i != b && i != c
					    && (std::find(targets.begin(), targets.end(), i)
					        == targets.end())) {
						helper = i;
						break;
					}
				}

				if (helper == qid_invalid) {
					std::cout << "[e] insufficient helper qubits for mapping, "
					             "break\n";
					return false;
				}

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_set::cx, d, targets[i]);
				}

				// R1-TOF(a, b, helper)
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, a, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);

				// S-R2-TOF(c3, helper, target)
				dest.add_gate(gate_set::hadamard, d);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, c, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, c, helper);
				dest.add_gate(gate_set::t, helper);

				// R1-TOF^-1(a, b, helper)
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, a, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);

				// S-R2-TOF^-1(c3, helper, target)
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, c, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, c, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::hadamard, d);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_set::cx, d, targets[i]);
				}
			} break;

			case 4u: {
				uint32_t controls[4];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				const auto a = controls[0];
				const auto b = controls[1];
				const auto control = controls[2];
				const auto d = controls[3];
				const auto e = targets[0];

				auto helper = qid_invalid;
				for (auto i = 0u; i < dest.num_qubits(); ++i) {
					if (i != a && i != b && i != control && i != d
					    && (std::find(targets.begin(), targets.end(), i)
					        == targets.end())) {
						helper = i;
						break;
					}
				}

				if (helper == qid_invalid) {
					std::cout << "[e] no sufficient helper line "
					             "found for mapping, break\n";
					return false;
				}

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_set::cx, e, targets[i]);
				}

				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, control, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::cx, a, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, a, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, control, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::hadamard, e);
				dest.add_gate(gate_set::cx, e, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, e, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, control, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, a, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, b, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, a, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, control, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::hadamard, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, e, helper);
				dest.add_gate(gate_set::t_dagger, helper);
				dest.add_gate(gate_set::cx, d, helper);
				dest.add_gate(gate_set::t, helper);
				dest.add_gate(gate_set::cx, e, helper);
				dest.add_gate(gate_set::hadamard, e);

				for (auto i = 1u; i < targets.size(); ++i) {
					dest.add_gate(gate_set::cx, e, targets[i]);
				}
			} break;
			}
			return true;
		} else if (gate.op().is(gate_set::mcz)) {
			if (gate.num_controls() == 2) {
				uint32_t controls[2];
				auto* p = controls;
				std::vector<uint32_t> targets;
				gate.foreach_control([&](auto control) { *p++ = control; });
				gate.foreach_target([&](auto target) { targets.push_back(target); });

				dest.add_gate(gate_set::cx, controls[1], targets[0]);
				dest.add_gate(gate_set::t_dagger, targets[0]);
				dest.add_gate(gate_set::cx, controls[0], targets[0]);
				dest.add_gate(gate_set::t, targets[0]);
				dest.add_gate(gate_set::cx, controls[1], targets[0]);
				dest.add_gate(gate_set::t_dagger, targets[0]);
				dest.add_gate(gate_set::cx, controls[0], targets[0]);
				dest.add_gate(gate_set::t, targets[0]);
				dest.add_gate(gate_set::cx, controls[0], controls[1]);
				dest.add_gate(gate_set::t_dagger, controls[1]);
				dest.add_gate(gate_set::cx, controls[0], controls[1]);
				dest.add_gate(gate_set::t, controls[1]);
				dest.add_gate(gate_set::t, controls[0]);
				return true;
			}
		}
		return false;
	};

	constexpr auto num_ancillae = 1u;
	Network dest;
	rewrite_network(dest, src, gate_rewriter, num_ancillae);
	return dest;
}

} // namespace tweedledum