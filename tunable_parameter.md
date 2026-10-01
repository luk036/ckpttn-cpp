# Tunable Parameters in `ckpttn-cpp`

A complete reference of every user-tunable parameter of the algorithms in this
project, together with its default value and source location.

## Key finding

`ckpttn-cpp` has **no runtime `Options` object**. Tunables are split across:

1. **Constructor parameters** (`bal_tol`, `num_parts`)
2. **Setters** (`set_limitsize`)
3. **Compile-time constants** (`constexpr`, mostly in `FMPmrConfig.hpp` and the
   manager headers)
4. **CLI options** (`standalone/source/main.cpp`, via `cxxopts`)
5. **One public mutable runtime flag** (`special_handle_2pin_nets`)

The only "config" file is `include/ckpttn/FMPmrConfig.hpp`, which holds two
compile-time caps.

---

## 1. Core partition tunables

| Parameter | Default | Location | Notes |
| --- | --- | --- | --- |
| `bal_tol` | required (no default) | `include/ckpttn/FMConstrMgr.hpp:53`, `MLPartMgr.hpp:54`, `MidLvlPartMgr.hpp:37` | `lowerbound = round((1 - bal_tol) * total_weight / num_parts)` (`source/FMConstrMgr.cpp:34`). |
| `num_parts` | `2` | `FMConstrMgr.hpp:53`, `FMBiGainMgr.hpp:37`, `FMPartMgr.hpp:57` | `FMBiConstrMgr` forces 2 regardless of the argument (`FMBiConstrMgr.hpp:25,33`). |
| `limitsize` | `50U` | `MLPartMgr.hpp:42`, `MLMidLvlPartMgr.hpp:64`, `MLMidLvlKWayPartMgr.hpp:55` | Set via `set_limitsize(size_t)`. |
| `max_passes` (FM optimize loop) | `100` (hardcoded) | `source/PartMgrBase.cpp:159` | `for (int iter = 0; iter < 100; ++iter)`. |
| contraction ratio | `> 3/2` reduction (hardcoded) | `source/MLPartMgr.cpp:66` | `hgr2->modules * 3 / 2 < hyprgraph.modules`. |

### Constructors

| API | Signature | Default |
| --- | --- | --- |
| `MLPartMgr` | `(bal_tol)` / `(bal_tol, num_parts)` | `num_parts = 2` |
| `FMConstrMgr` | `(hyprgraph, bal_tol)` / `(hyprgraph, bal_tol, num_parts)` | `num_parts = 2` |
| `FMPartMgr` | `(hyprgraph, gain_mgr, constr_mgr)` / `(..., num_parts)` | `num_parts = 2` |
| `PartMgrBase` / `NNPartMgr` | `(..., num_parts)` | required |
| `FMBiGainMgr` | `(hyprgraph)` / `(hyprgraph, num_parts)` | always 2 |
| `FMKWayGainMgr` | `(hyprgraph, num_parts)` | required |

---

## 2. Mid-level exhaustive tunables

| Constant | Default | Location |
| --- | --- | --- |
| `MidLvlKWayPartMgr::max_passes` | `5` | `include/ckpttn/MidLvlKWayPartMgr.hpp:47` |
| `MidLvlKWayPartMgr::max_pair_modules` | `15` | `include/ckpttn/MidLvlKWayPartMgr.hpp:49` |
| `MLMidLvlPartMgr::exhaustive_limit` | `25U` | `include/ckpttn/MLMidLvlPartMgr.hpp:66` |
| `MLMidLvlPartMgr::limitsize` | `50U` (settable) | `include/ckpttn/MLMidLvlPartMgr.hpp:64` |
| `MLMidLvlKWayPartMgr::base_exhaustive` | `25U` | `include/ckpttn/MLMidLvlKWayPartMgr.hpp:57` |
| `MLMidLvlKWayPartMgr::limitsize_` | `50U` (settable) | `include/ckpttn/MLMidLvlKWayPartMgr.hpp:55` |

`MLMidLvlKWayPartMgr` derives its threshold dynamically:
`exhaustive_limit = (base_exhaustive * num_parts) / 2`
(`source/MLMidLvlKWayPartMgr.cpp:44`).

---

## 3. FM algorithm compile-time caps

File: `include/ckpttn/FMPmrConfig.hpp`.

| Constant | Default | Location |
| --- | --- | --- |
| `FM_MAX_NUM_PARTITIONS` | `255U` | `FMPmrConfig.hpp:25` |
| `FM_MAX_DEGREE` (max net size) | `500U` | `FMPmrConfig.hpp:27` |

Also in this file: `namespace FMPmr = std::pmr` (allocator backend,
`FMPmrConfig.hpp:22`).

---

## 4. Gain-calculator performance tunables

| Parameter | Default | Location | Notes |
| --- | --- | --- | --- |
| `FMBiGainCalc::stack_buf_size` | `32768` (32 KiB, compile-time) | `FMBiGainCalc.hpp:56` | PMR stack buffer. |
| `FMKWayGainCalc::stack_buf_size` | `65536` (64 KiB, compile-time) | `FMKWayGainCalc.hpp:52` | PMR stack buffer. |
| `special_handle_2pin_nets` | `true` (public, mutable) | `FMBiGainCalc.hpp:68`, `FMKWayGainCalc.hpp:78` | Marked `@TODO should be template parameter`. |
| gain bucket range | `±(num_parts - 1) * max_degree` (derived) | `source/FMGainMgr.cpp:34-36` | |

---

## 5. Clustering constants

File: `source/min_cover.cpp`.

| Constant | Default | Location |
| --- | --- | --- |
| `LOW_PIN_NET_THRESHOLD` | `5` | `min_cover.cpp:19` |
| `MINHASH_SIG_SIZE` | `64` | `min_cover.cpp:20` |
| `MINHASH_SIMILARITY` | `0.8` | `min_cover.cpp:21` |
| `MINHASH_MAX_DEGREE` | `200` | `min_cover.cpp:22` |

---

## 6. CLI options

File: `standalone/source/main.cpp` (cxxopts).

| Option | Default | Location |
| --- | --- | --- |
| `k` | `2` | `main.cpp:113, 136` |
| `epsilon` | `0.05` (values > 1 divided by 100) | `main.cpp:114, 138, 257-259` |
| `--input-format` | `"auto"` | `main.cpp:116, 141` |
| `--fixed` | `""` | `main.cpp:117, 143` |
| `--output` | `""` (stdout) | `main.cpp:119, 146` |
| `--output-format` | `"hmetis"` | `main.cpp:120, 148` |
| `--quiet` | `false` | `main.cpp:121, 149` |
| `--preset` | `"default"` | `main.cpp:123, 153` |
| `--objective` | `"cut"` (parsed, unused) | `main.cpp:124, 155` |
| `--mode` | `"recursive"` (`direct` = NN) | `main.cpp:125, 157` |
| `--threads` (number of starts) | `1` | `main.cpp:126, 159` |
| `--seed` | `0` (0 = random device) | `main.cpp:128, 162` |
| `--verbose` | `false` | `main.cpp:129, 162` |
| `--time-limit` | `0.0` (parsed, unused) | `main.cpp:130, 166` |
| `--max-quality` | `0` (parsed, unused) | `main.cpp:131, 168` |

### Presets (`main.cpp:36-51`)

| Preset | `balance_tolerance` | `use_recursive` |
| --- | --- | --- |
| `default` | `0.03` | `true` |
| `quality` | `0.01` | `false` |
| `highest_quality` | `0.005` | `false` |
| `deterministic` | `0.03` | `true` |
| `large_k` | `0.03` | `true` |

The CLI overrides the preset tolerance with `epsilon` (`main.cpp:266`).

---

## 7. Logging

File: `source/logger.cpp`. Fixed (not parameterized): file `"ckpttn.log"`,
level `spdlog::level::info`, flush on info (`logger.cpp:22-25`).

---

## 8. Gray-code utility (standalone demo)

File: `middle/main_cli.cpp`.

| Option | Default | Location |
| --- | --- | --- |
| `-n{}` | mandatory | `main_cli.cpp:88-98` |
| `-l{}` (steps) | `-1` (full cycle) | `main_cli.cpp:75, 99-107` |
| `-v{bitstring}` | all-ones | `main_cli.cpp:76, 175-179` |
| `-s{0,1}` (store) | `1` | `main_cli.cpp:78, 142-150` |
| `-p{0,1}` (print flips) | `0` | `main_cli.cpp:79, 151-159` |

---

## 9. Fixed values in tests and benchmarks

| Source | Constant | Value |
| --- | --- | --- |
| `test/source/test_common.hpp:24,42` | `bal_tol` | `0.4` |
| `test/source/test_MidLvlPartMgr.cpp`, `test_MLMidLvlPartMgr.cpp` | `bal_tol` | `0.45` |
| `test/source/test_MidLvlKWayPartMgr.cpp:13`, `test_MLMidLvlKWayPartMgr.cpp:16` | `bal_tol`, `num_parts` | `0.4`, `3` |
| `test/source/test_MLPartMgr.cpp:35,51`, `test_MLNNPartMgr.cpp:34,50`, `test_MLPartMgr_yosys.cpp` | `bal_tol`, `num_parts` | `0.3` / `0.4`, `3` |
| `test/source/test_MLPartMgr.cpp:70`, `test_MLNNPartMgr.cpp:69`, `test_MLMidLvlPartMgr.cpp:31` | `limitsize` | `10` / `500` / `200` |
| `bench/BenchFMNN.cpp:11,13` | `BAL_TOL`, `SEEDS` | `0.45`, `{0, 1, 2, 3, 4}` |
| `bench/BM_ibm01.cpp:18-19` | `SEED`, `RUNS`, `BAL_TOL` | `47`, `5`, `0.45` |
| `bench/BM_ibm18_stress.cpp:25-26` | `bal_tol`, `limitsize` | `0.45`, `24000` |
| `bench/FMBi_2pin_nets.cpp:33`, `bench/FMBi_p1.cpp:20` | `bal_tol` | `0.45` |
| `bench/FMKWay_2pin_nets.cpp:35` | `bal_tol` | `0.4` |

---

## Summary of genuine tunables

| Tunable | Default | Kind | Location |
| --- | --- | --- | --- |
| `bal_tol` | required (CLI preset: `0.03` / `0.01` / `0.005`) | ctor / CLI | `include/ckpttn/FMConstrMgr.hpp:53`, `standalone/source/main.cpp:36-51` |
| `num_parts` | `2` | ctor / CLI | `include/ckpttn/FMConstrMgr.hpp:53`, `standalone/source/main.cpp:136` |
| `limitsize` | `50U` | setter | `include/ckpttn/MLPartMgr.hpp:42`, `include/ckpttn/MLMidLvl*PartMgr.hpp` |
| `max_passes` (FM optimize) | `100` (hardcoded) | const | `source/PartMgrBase.cpp:159` |
| `MidLvlKWayPartMgr::max_passes` | `5` | const | `include/ckpttn/MidLvlKWayPartMgr.hpp:47` |
| `MidLvlKWayPartMgr::max_pair_modules` | `15` | const | `include/ckpttn/MidLvlKWayPartMgr.hpp:49` |
| `exhaustive_limit` / `base_exhaustive` | `25U` | const | `include/ckpttn/MLMidLvlPartMgr.hpp:66`, `include/ckpttn/MLMidLvlKWayPartMgr.hpp:57` |
| `FM_MAX_NUM_PARTITIONS` | `255U` | const | `include/ckpttn/FMPmrConfig.hpp:25` |
| `FM_MAX_DEGREE` | `500U` | const | `include/ckpttn/FMPmrConfig.hpp:27` |
| `stack_buf_size` | `32768` (bi) / `65536` (k-way) | const | `include/ckpttn/FMBiGainCalc.hpp:56`, `include/ckpttn/FMKWayGainCalc.hpp:52` |
| `special_handle_2pin_nets` | `true` | public flag | `include/ckpttn/FMBiGainCalc.hpp:68`, `include/ckpttn/FMKWayGainCalc.hpp:78` |
| `LOW_PIN_NET_THRESHOLD` | `5` | const | `source/min_cover.cpp:19` |
| `MINHASH_SIG_SIZE` | `64` | const | `source/min_cover.cpp:20` |
| `MINHASH_SIMILARITY` | `0.8` | const | `source/min_cover.cpp:21` |
| `MINHASH_MAX_DEGREE` | `200` | const | `source/min_cover.cpp:22` |
| CLI `k` / `epsilon` / `--threads` / `--seed` / `--mode` / `--preset` | `2` / `0.05` / `1` / `0` / `recursive` / `default` | CLI | `standalone/source/main.cpp:136, 138, 159, 162, 157, 153` |

Everything else is problem data (hypergraph, module weights, fixed-module set,
initial partition) or a compile-time constant. As in the Python port, three CLI
flags (`--objective`, `--time-limit`, `--max-quality`) are parsed but never used.
