/**
 * @file LegalCheck.hpp
 * @brief Result of a partition legality check
 */

#pragma once

/**
 * @brief Check if the move of v can be satisfied, get better, or not satisfied
 */
enum class LegalCheck { NotSatisfied, GetBetter, AllSatisfied };
