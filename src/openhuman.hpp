// OpenHuman v0.1 core: synthetic lattice tissue. Arbitrary units; NOT calibrated physiology.
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <locale>
#include <type_traits>

namespace oh {

enum : uint8_t { EMPTY = 0, ALIVE = 1, DEAD = 2 };
enum { GLU = 0, OXY = 1, WASTE = 2, NSPEC = 3 };
constexpr uint32_t CKPT_VERSION = 2;

// Shared rules; checkpoint fields are serialized individually as decimal text.
struct Params {
  int64_t nx = 96, ny = 48, seed_depth = 4;
  uint64_t seed = 1;
  double dt = 0.1;
  double D[NSPEC] = {1.0, 2.0, 1.0};
  double supply[NSPEC] = {5.0, 1.0, 0.0};
  double k_exchange = 1.0;
  double Vg = 0.02, Kg = 0.5, Vo = 0.12, Ko = 0.05, glyco_boost = 1.0;
  double atp_ox = 30.0, atp_ferm = 2.0, o2_per_glu = 6.0, waste_per_glu = 2.0;
  double maintenance = 0.03, E_max = 1.0, E_div = 0.8, E_div_cost = 0.2, r_div = 0.05;
  double W_tox = 4.0, k_damage = 0.01, k_repair = 0.02, k_clear = 0.01;
  double seed_fraction = 0.5;
};

struct Event { double t; std::string key; double value; };  // at time t, set a double param

struct Ledger {
  double mass0[NSPEC]{}, supplied[NSPEC]{}, consumed[NSPEC]{}, produced[NSPEC]{};
  int64_t births = 0, starved = 0, poisoned = 0, cleared = 0;
};

inline double* dfield(Params& p, const std::string& k) {
  const struct { const char* n; double* f; } t[] = {
    {"dt", &p.dt}, {"D_glucose", &p.D[GLU]}, {"D_oxygen", &p.D[OXY]}, {"D_waste", &p.D[WASTE]},
    {"supply_glucose", &p.supply[GLU]}, {"supply_oxygen", &p.supply[OXY]}, {"supply_waste", &p.supply[WASTE]},
    {"k_exchange", &p.k_exchange}, {"Vg", &p.Vg}, {"Kg", &p.Kg}, {"Vo", &p.Vo}, {"Ko", &p.Ko},
    {"glyco_boost", &p.glyco_boost}, {"atp_ox", &p.atp_ox}, {"atp_ferm", &p.atp_ferm},
    {"o2_per_glu", &p.o2_per_glu}, {"waste_per_glu", &p.waste_per_glu}, {"maintenance", &p.maintenance},
    {"E_max", &p.E_max}, {"E_div", &p.E_div}, {"E_div_cost", &p.E_div_cost}, {"r_div", &p.r_div},
    {"W_tox", &p.W_tox}, {"k_damage", &p.k_damage}, {"k_repair", &p.k_repair}, {"k_clear", &p.k_clear},
    {"seed_fraction", &p.seed_fraction}};
  for (auto& e : t) if (k == e.n) return e.f;
  return nullptr;
}

inline const std::vector<std::string>& parameter_names() {
  static const std::vector<std::string> names = {"dt", "D_glucose", "D_oxygen", "D_waste", "supply_glucose", "supply_oxygen", "supply_waste", "k_exchange", "Vg", "Kg", "Vo", "Ko", "glyco_boost", "atp_ox", "atp_ferm", "o2_per_glu", "waste_per_glu", "maintenance", "E_max", "E_div", "E_div_cost", "r_div", "W_tox", "k_damage", "k_repair", "k_clear", "seed_fraction"};
  return names;
}

template<class T> inline T number(const std::string& text) {
  std::istringstream in(text); in.imbue(std::locale::classic()); T value{};
  if (!(in >> value) || !(in >> std::ws).eof()) throw std::runtime_error("invalid number: " + text);
  if constexpr (std::is_unsigned_v<T>) if (text.find('-') != std::string::npos) throw std::runtime_error("negative unsigned value");
  return value;
}

inline void set_kv(Params& p, std::vector<Event>& ev, const std::string& k, const std::string& v) {
  if (k == "event") {  // event = <time> <param> <value>
    std::istringstream ss(v); double t, val; std::string name, trailing;
    if (!(ss >> t >> name >> val)) throw std::runtime_error("event needs: <time> <param> <value>");
    double* f = dfield(p, name);
    if (!f || name == "dt") throw std::runtime_error("event: unknown or forbidden param '" + name + "'");
    if (!std::isfinite(t) || t < 0 || !std::isfinite(val) || (ss >> trailing))
      throw std::runtime_error("invalid event");
    ev.push_back({t, name, val});
  } else if (k == "nx") p.nx = number<int64_t>(v);
  else if (k == "ny") p.ny = number<int64_t>(v);
  else if (k == "seed_depth") p.seed_depth = number<int64_t>(v);
  else if (k == "seed") p.seed = number<uint64_t>(v);
  else if (double* f = dfield(p, k)) *f = number<double>(v);
  else throw std::runtime_error("unknown key '" + k + "'");
}

inline std::string trim(const std::string& s) {
  auto a = s.find_first_not_of(" \t\r"), b = s.find_last_not_of(" \t\r");
  return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

inline void load_config(const std::string& path, Params& p, std::vector<Event>& ev) {
  std::ifstream f(path);
  if (!f) throw std::runtime_error("cannot open config: " + path);
  std::string line; int ln = 0;
  while (std::getline(f, line)) {
    ++ln;
    auto h = line.find('#'); if (h != std::string::npos) line.erase(h);
    if (trim(line).empty()) continue;
    auto eq = line.find('=');
    try {
      if (eq == std::string::npos) throw std::runtime_error("expected key = value");
      set_kv(p, ev, trim(line.substr(0, eq)), trim(line.substr(eq + 1)));
    } catch (const std::exception& e) {
      throw std::runtime_error(path + ":" + std::to_string(ln) + ": " + e.what());
    }
  }
}

inline void validate(const Params& p) {
  auto req = [](bool ok, const char* m) { if (!ok) throw std::runtime_error(std::string("invalid parameters: ") + m); };
  req(p.nx >= 2 && p.ny >= 2 && p.nx <= 100000 && p.ny <= 100000 && p.nx * p.ny <= 100000000, "grid size out of range");
  Params copy = p;
  for (const auto& name : parameter_names()) req(std::isfinite(*dfield(copy, name)), "parameters must be finite");
  req(p.atp_ox >= 0 && p.atp_ferm >= 0 && p.waste_per_glu >= 0 && p.maintenance >= 0 && p.W_tox >= 0, "yields, maintenance and toxicity threshold must be nonnegative");
  req(p.dt > 0, "dt must be > 0");
  for (int s = 0; s < NSPEC; ++s) {
    req(p.D[s] >= 0 && p.D[s] * p.dt <= 0.25, "need 0 <= D*dt <= 0.25 (explicit diffusion stability, dx = 1)");
    req(p.supply[s] >= 0, "supply must be >= 0");
  }
  req(p.k_exchange >= 0 && p.k_exchange * p.dt <= 1, "need 0 <= k_exchange*dt <= 1");
  req(p.Kg > 0 && p.Ko > 0 && p.Vg >= 0 && p.Vo >= 0 && p.glyco_boost >= 0, "uptake constants out of range");
  req(p.o2_per_glu > 0 && p.E_max > 0 && p.E_div > p.E_div_cost && p.E_div <= p.E_max && p.E_div_cost >= 0, "energy constants out of range");
  req(p.k_damage >= 0 && p.k_repair >= 0 && p.k_repair * p.dt <= 1, "damage constants out of range");
  req(p.r_div >= 0 && p.k_clear >= 0, "rates must be >= 0");
  req(p.seed_depth >= 0 && p.seed_fraction >= 0 && p.seed_fraction <= 1, "seeding out of range");
}

inline int64_t event_step(const Event& e, double dt) {
  double q = e.t / dt;
  if (!std::isfinite(q) || q < 0 || q >= static_cast<double>(INT64_MAX))
    throw std::runtime_error("event time out of range");
  if (std::abs(q - std::round(q)) > 1e-8 * std::max(1.0, std::abs(q)))
    throw std::runtime_error("event time must align with dt");
  return static_cast<int64_t>(std::round(q));
}
inline void validate_events(const Params& p, const std::vector<Event>& events) {
  Params trial = p;
  auto ordered = events;
  std::stable_sort(ordered.begin(), ordered.end(), [](const Event& a, const Event& b) { return a.t < b.t; });
  for (const auto& e : ordered) {
    event_step(e, p.dt);
    double* field = dfield(trial, e.key);
    if (!field || e.key == "dt" || !std::isfinite(e.value)) throw std::runtime_error("invalid event parameter");
    *field = e.value;
    validate(trial);
  }
}

// Counter-based RNG: value depends only on (seed, step, voxel, stream), not on execution order.
inline uint64_t mix(uint64_t x) {
  x += 0x9E3779B97F4A7C15ULL; x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
  x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL; return x ^ (x >> 31);
}
inline uint64_t rnd(uint64_t seed, uint64_t step, uint64_t i, uint64_t stream) {
  return mix(mix(mix(seed ^ (stream * 0xD6E8FEB86659FD93ULL)) + step) + i);
}
inline double u01(uint64_t h) { return (double)(h >> 11) * (1.0 / 9007199254740992.0); }

struct Organism {
  Params p; std::vector<Event> events; int64_t step = 0; Ledger led;
  std::array<std::vector<double>, NSPEC> c;          // concentration fields
  std::vector<uint8_t> state; std::vector<double> energy, damage;  // per-voxel cell state (SoA)
  std::vector<double> tmp; std::vector<int64_t> proposal;          // scratch
  std::FILE* evlog = nullptr;

  int64_t n() const { return p.nx * p.ny; }
  double mass(int s) const { double m = 0; for (double v : c[s]) m += v; return m; }

  void init() {
    validate(p); validate_events(p, events); led = {};
    const size_t N = (size_t)n();
    for (int s = 0; s < NSPEC; ++s) c[s].assign(N, p.supply[s]);
    state.assign(N, (uint8_t)EMPTY); energy.assign(N, 0.0); damage.assign(N, 0.0);
    for (int64_t y = 0; y < p.ny; ++y)
      for (int64_t x = 0; x < std::min(p.seed_depth, p.nx); ++x) {
        int64_t i = y * p.nx + x;
        if (u01(rnd(p.seed, 0, (uint64_t)i, 0)) < p.seed_fraction) { state[i] = ALIVE; energy[i] = 0.5 * p.E_max; }
      }
    for (int s = 0; s < NSPEC; ++s) led.mass0[s] = mass(s);
    tmp.resize(N); proposal.resize(N); step = 0;
  }

  void step_once() { if (step == INT64_MAX) throw std::runtime_error("step overflow"); apply_events(); exchange(); metabolism(); fate(); diffuse(); ++step; }

  void apply_events() {
    bool changed = false;
    for (const Event& e : events)
      if (event_step(e, p.dt) == step) { *dfield(p, e.key) = e.value; changed = true; }
    if (changed) validate(p);
  }

  void exchange() {  // column x = 0 exchanges with a reservoir at the supply concentration
    for (int s = 0; s < NSPEC; ++s)
      for (int64_t y = 0; y < p.ny; ++y) {
        double& v = c[s][y * p.nx];
        double d = p.k_exchange * p.dt * (p.supply[s] - v);
        v += d; led.supplied[s] += d;
      }
  }

  void metabolism() {
    const int64_t N = n(); double cg = 0, co = 0, pw = 0;
    for (int64_t i = 0; i < N; ++i) {
      if (state[i] != ALIVE) continue;
      const double G = c[GLU][i], O = c[OXY][i];
      const double boost = 1.0 + p.glyco_boost * (1.0 - std::clamp(energy[i] / p.E_max, 0.0, 1.0));  // low energy -> more uptake
      const double ug = std::min(G, p.dt * p.Vg * boost * G / (p.Kg + G));
      const double uo_cap = std::min(O, p.dt * p.Vo * O / (p.Ko + O));
      const double g_ox = std::min(ug, uo_cap / p.o2_per_glu);   // glucose oxidised (oxygen-limited)
      const double g_ferm = ug - g_ox;                            // remainder fermented -> waste
      const double uo = std::min(uo_cap, g_ox * p.o2_per_glu);
      const double w = g_ferm * p.waste_per_glu;
      c[GLU][i] = G - ug; c[OXY][i] = O - uo; c[WASTE][i] += w;
      cg += ug; co += uo; pw += w;
      energy[i] = std::min(p.E_max, energy[i] + p.atp_ox * g_ox + p.atp_ferm * g_ferm - p.maintenance * p.dt);
      damage[i] += p.dt * (p.k_damage * std::max(0.0, c[WASTE][i] - p.W_tox) - p.k_repair * damage[i]);
    }
    led.consumed[GLU] += cg; led.consumed[OXY] += co; led.produced[WASTE] += pw;
  }

  int neighbors(int64_t i, int64_t out[4]) const {
    int k = 0; const int64_t x = i % p.nx, y = i / p.nx;
    if (x > 0) out[k++] = i - 1;
    if (x < p.nx - 1) out[k++] = i + 1;
    if (y > 0) out[k++] = i - p.nx;
    if (y < p.ny - 1) out[k++] = i + p.nx;
    return k;
  }

  void log(const char* what, int64_t i) {
    if (evlog) std::fprintf(evlog, "%lld,%s,%lld,%lld\n", (long long)step, what, (long long)(i % p.nx), (long long)(i / p.nx));
  }

  void fate() {
    const int64_t N = n(); const uint64_t st = (uint64_t)step;
    const double p_div = 1 - std::exp(-p.r_div * p.dt), p_clear = 1 - std::exp(-p.k_clear * p.dt);
    for (int64_t i = 0; i < N; ++i) {  // death and clearance
      if (state[i] == ALIVE) {
        if (energy[i] <= 0) { state[i] = DEAD; energy[i] = 0; ++led.starved; log("death_energy", i); }
        else if (damage[i] >= 1) { state[i] = DEAD; energy[i] = 0; ++led.poisoned; log("death_damage", i); }
      } else if (state[i] == DEAD && u01(rnd(p.seed, st, (uint64_t)i, 1)) < p_clear) {
        state[i] = EMPTY; damage[i] = 0; ++led.cleared; log("cleared", i);
      }
    }
    int64_t nb[4];
    for (int64_t i = 0; i < N; ++i) {  // each eligible cell proposes one empty neighbour
      proposal[i] = -1;
      if (state[i] != ALIVE || energy[i] < p.E_div || u01(rnd(p.seed, st, (uint64_t)i, 2)) >= p_div) continue;
      int64_t empty[4]; int m = 0;
      for (int k = neighbors(i, nb); k-- > 0;) if (state[nb[k]] == EMPTY) empty[m++] = nb[k];
      if (m) proposal[i] = empty[rnd(p.seed, st, (uint64_t)i, 3) % (uint64_t)m];
    }
    for (int64_t v = 0; v < N; ++v) {  // each empty voxel accepts at most one proposer
      if (state[v] != EMPTY) continue;
      int64_t parent = -1; uint64_t best = 0;
      for (int k = neighbors(v, nb); k-- > 0;) {
        const int64_t q = nb[k];
        if (proposal[q] != v) continue;
        const uint64_t h = rnd(p.seed, st, (uint64_t)q, 4);
        if (parent < 0 || h < best) { parent = q; best = h; }
      }
      if (parent < 0) continue;
      const double e = 0.5 * (energy[parent] - p.E_div_cost);
      energy[parent] = energy[v] = e; damage[v] = damage[parent]; state[v] = ALIVE;
      ++led.births; log("division", v);
    }
  }

  void diffuse() {  // explicit FTCS, flux form, no-flux walls
    const int64_t nx = p.nx, ny = p.ny;
    for (int s = 0; s < NSPEC; ++s) {
      const double a = p.D[s] * p.dt;
      if (a == 0) continue;
      const std::vector<double>& src = c[s];
      #pragma omp parallel for schedule(static)
      for (int64_t y = 0; y < ny; ++y)
        for (int64_t x = 0; x < nx; ++x) {
          const int64_t i = y * nx + x; const double v = src[i]; double acc = 0;
          if (x > 0) acc += src[i - 1] - v;
          if (x < nx - 1) acc += src[i + 1] - v;
          if (y > 0) acc += src[i - nx] - v;
          if (y < ny - 1) acc += src[i + nx] - v;
          tmp[i] = v + a * acc;
        }
      c[s].swap(tmp);
    }
  }

  struct Metrics { int64_t alive = 0, dead = 0; double mean_E = 0, mean_damage = 0, mass[NSPEC], resid[NSPEC], min_conc = 0; };
  Metrics metrics() const {
    Metrics m;
    for (int64_t i = 0; i < n(); ++i) {
      if (state[i] == ALIVE) { ++m.alive; m.mean_E += energy[i]; m.mean_damage += damage[i]; }
      else if (state[i] == DEAD) ++m.dead;
    }
    if (m.alive) { m.mean_E /= (double)m.alive; m.mean_damage /= (double)m.alive; }
    m.min_conc = c[0][0];
    for (int s = 0; s < NSPEC; ++s) {
      m.mass[s] = mass(s);
      m.resid[s] = m.mass[s] - (led.mass0[s] + led.supplied[s] - led.consumed[s] + led.produced[s]);
      for (double v : c[s]) m.min_conc = std::min(m.min_conc, v);
    }
    return m;
  }

  uint64_t hash() const {  // FNV-1a over step, cells and fields (not parameters/events/ledger)
    uint64_t h = 14695981039346656037ULL;
    auto eat = [&h](const void* d, size_t len) {
      auto b = (const unsigned char*)d;
      for (size_t i = 0; i < len; ++i) { h ^= b[i]; h *= 1099511628211ULL; }
    };
    eat(&step, sizeof step); eat(state.data(), state.size());
    eat(energy.data(), energy.size() * 8); eat(damage.data(), damage.size() * 8);
    for (auto& f : c) eat(f.data(), f.size() * 8);
    return h;
  }

  // Versioned decimal representation: no padding, raw pointers, or byte offsets.
  // 17 significant digits round-trip IEEE-754 doubles; simulation equivalence
  // across different floating-point implementations is not promised.
  void save(const std::string& path) const {
    std::ofstream out(path);
    out.imbue(std::locale::classic()); out << std::setprecision(17);
    if (!out) throw std::runtime_error("cannot write checkpoint: " + path);
    out << "OHCK " << CKPT_VERSION << '\n';
    out << p.nx << ' ' << p.ny << ' ' << p.seed_depth << ' ' << p.seed << '\n';
    Params copy = p;
    for (const auto& name : parameter_names()) out << name << ' ' << *dfield(copy, name) << '\n';
    out << step << '\n';
    for (int j = 0; j < NSPEC; ++j)
      out << led.mass0[j] << ' ' << led.supplied[j] << ' ' << led.consumed[j] << ' ' << led.produced[j] << '\n';
    out << led.births << ' ' << led.starved << ' ' << led.poisoned << ' ' << led.cleared << '\n';
    out << events.size() << '\n';
    for (const auto& e : events) out << e.t << ' ' << e.key << ' ' << e.value << '\n';
    for (int64_t i = 0; i < n(); ++i)
      out << static_cast<int>(state[i]) << ' ' << energy[i] << ' ' << damage[i] << ' '
          << c[0][i] << ' ' << c[1][i] << ' ' << c[2][i] << '\n';
    out.close();
    if (!out) throw std::runtime_error("checkpoint write failed");
  }

  void load(const std::string& path) {
    std::ifstream in(path); in.imbue(std::locale::classic());
    if (!in) throw std::runtime_error("cannot open checkpoint: " + path);
    auto read = [&in](auto& value) {
      if (!(in >> value)) throw std::runtime_error("checkpoint truncated or invalid");
    };
    Organism next; std::string magic; uint32_t ver;
    read(magic); read(ver);
    if (magic != "OHCK" || ver != CKPT_VERSION) throw std::runtime_error("unsupported checkpoint");
    read(next.p.nx); read(next.p.ny); read(next.p.seed_depth); read(next.p.seed);
    for (const auto& expected : parameter_names()) {
      std::string key; read(key);
      if (key != expected) throw std::runtime_error("checkpoint parameter mismatch");
      read(*dfield(next.p, key));
    }
    validate(next.p); read(next.step);
    if (next.step < 0) throw std::runtime_error("negative checkpoint step");
    for (int j = 0; j < NSPEC; ++j) {
      read(next.led.mass0[j]); read(next.led.supplied[j]); read(next.led.consumed[j]); read(next.led.produced[j]);
      if (!std::isfinite(next.led.mass0[j]) || !std::isfinite(next.led.supplied[j]) ||
          !std::isfinite(next.led.consumed[j]) || !std::isfinite(next.led.produced[j]) ||
          next.led.mass0[j] < 0 || next.led.consumed[j] < 0 || next.led.produced[j] < 0)
        throw std::runtime_error("invalid checkpoint ledger");
    }
    read(next.led.births); read(next.led.starved); read(next.led.poisoned); read(next.led.cleared);
    if (next.led.births < 0 || next.led.starved < 0 || next.led.poisoned < 0 || next.led.cleared < 0)
      throw std::runtime_error("negative checkpoint counter");
    int64_t count; read(count);
    if (count < 0 || count > 10000) throw std::runtime_error("invalid event count");
    for (int64_t k = 0; k < count; ++k) {
      Event e; read(e.t); read(e.key); read(e.value); next.events.push_back(e);
    }
    validate_events(next.p, next.events);
    const size_t N = static_cast<size_t>(next.n());
    for (auto& field : next.c) field.resize(N);
    next.state.resize(N); next.energy.resize(N); next.damage.resize(N);
    for (size_t i = 0; i < N; ++i) {
      int state_value; read(state_value); read(next.energy[i]); read(next.damage[i]);
      if (state_value < EMPTY || state_value > DEAD || !std::isfinite(next.energy[i]) ||
          next.energy[i] < 0 || !std::isfinite(next.damage[i]) || next.damage[i] < 0)
        throw std::runtime_error("invalid checkpoint cell");
      next.state[i] = static_cast<uint8_t>(state_value);
      for (auto& field : next.c) {
        read(field[i]);
        if (!std::isfinite(field[i]) || field[i] < 0) throw std::runtime_error("invalid checkpoint concentration");
      }
    }
    std::string trailing;
    if (in >> trailing) throw std::runtime_error("trailing checkpoint content");
    next.tmp.resize(N); next.proposal.resize(N);
    *this = std::move(next);
  }
};

}  // namespace oh
