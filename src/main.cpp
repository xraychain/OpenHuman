#include "openhuman.hpp"
#include <chrono>
#include <iostream>

int main(int argc, char** argv) {
  try {
    oh::Organism o;
    std::string cfg, resume, save, metrics, events, snapshot;
    std::vector<std::string> sets; long long steps = 1000, every = 10;
    for (int i = 1; i < argc; ++i) {
      const std::string a = argv[i];
      auto val = [&]() -> std::string {
        if (i + 1 >= argc) throw std::runtime_error("missing value for " + a);
        return argv[++i];
      };
      if (a == "--config") cfg = val(); else if (a == "--resume") resume = val();
      else if (a == "--save") save = val(); else if (a == "--metrics") metrics = val();
      else if (a == "--events") events = val(); else if (a == "--snapshot") snapshot = val();
      else if (a == "--set") sets.push_back(val());
      else if (a == "--steps") steps = oh::number<long long>(val()); else if (a == "--every") every = oh::number<long long>(val());
      else throw std::runtime_error("unknown argument " + a +
        "\nusage: openhuman [--config f] [--set key=value]... [--steps N] [--every K] [--metrics f.csv]"
        " [--events f.csv] [--snapshot f.csv] [--save f.ckpt] [--resume f.ckpt]");
    }
    if (steps < 0 || every < 1) throw std::runtime_error("--steps must be >= 0 and --every >= 1");

    if (!resume.empty() && (!cfg.empty() || !sets.empty())) throw std::runtime_error("resume cannot be combined with config or overrides");
    if (!resume.empty()) o.load(resume);
    else if (!cfg.empty()) oh::load_config(cfg, o.p, o.events);
    const auto nx = o.p.nx, ny = o.p.ny;
    for (auto& s : sets) {
      auto eq = s.find('=');
      if (eq == std::string::npos) throw std::runtime_error("--set expects key=value, got " + s);
      oh::set_kv(o.p, o.events, oh::trim(s.substr(0, eq)), oh::trim(s.substr(eq + 1)));
    }
    if (resume.empty()) o.init();
    else {
      if (o.p.nx != nx || o.p.ny != ny) throw std::runtime_error("cannot change grid size on resume");
      oh::validate(o.p);
    }

    std::FILE* mf = nullptr;
    if (!metrics.empty()) {
      mf = std::fopen(metrics.c_str(), "w");
      if (!mf) throw std::runtime_error("cannot write " + metrics);
      std::fprintf(mf, "step,time,alive,dead,births,deaths_energy,deaths_damage,cleared,mean_energy,mean_damage,"
                       "mass_glucose,mass_oxygen,mass_waste,resid_glucose,resid_oxygen,resid_waste,min_conc\n");
    }
    if (!events.empty()) {
      o.evlog = std::fopen(events.c_str(), "w");
      if (!o.evlog) throw std::runtime_error("cannot write " + events);
      std::fprintf(o.evlog, "step,event,x,y\n");
    }
    auto row = [&] {
      if (!mf) return;
      const auto m = o.metrics();
      std::fprintf(mf, "%lld,%.17g,%lld,%lld,%lld,%lld,%lld,%lld,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
        (long long)o.step, (double)o.step * o.p.dt, (long long)m.alive, (long long)m.dead, (long long)o.led.births,
        (long long)o.led.starved, (long long)o.led.poisoned, (long long)o.led.cleared, m.mean_E, m.mean_damage,
        m.mass[0], m.mass[1], m.mass[2], m.resid[0], m.resid[1], m.resid[2], m.min_conc);
    };

    const auto t0 = std::chrono::steady_clock::now();
    row();
    for (long long k = 0; k < steps; ++k) { o.step_once(); if (o.step % every == 0) row(); }
    if (o.step % every != 0) row();
    const double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    if (!save.empty()) o.save(save);
    if (!snapshot.empty()) {
      std::FILE* sf = std::fopen(snapshot.c_str(), "w");
      if (!sf) throw std::runtime_error("cannot write " + snapshot);
      std::fprintf(sf, "x,y,state,energy,damage,glucose,oxygen,waste\n");
      for (int64_t i = 0; i < o.n(); ++i)
        std::fprintf(sf, "%lld,%lld,%d,%.9g,%.9g,%.9g,%.9g,%.9g\n", (long long)(i % o.p.nx), (long long)(i / o.p.nx),
                     (int)o.state[i], o.energy[i], o.damage[i], o.c[0][i], o.c[1][i], o.c[2][i]);
      std::fclose(sf);
    }
    if (mf) std::fclose(mf);
    if (o.evlog) std::fclose(o.evlog);
    std::printf("step=%lld\nalive=%lld\nwall_s=%.4f\nsteps_per_s=%.1f\nhash=%016llx\n", (long long)o.step,
                (long long)o.metrics().alive, sec, steps > 0 && sec > 0 ? (double)steps / sec : 0.0,
                (unsigned long long)o.hash());
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
}
