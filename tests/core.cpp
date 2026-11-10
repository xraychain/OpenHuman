#include "openhuman.hpp"
#include <iostream>
#include <filesystem>

void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> void rejects(F f) { bool rejected = false; try { f(); } catch (const std::exception&) { rejected = true; } check(rejected, "expected rejection"); }
void close(double a, double b, double tolerance = 1e-10) { check(std::abs(a-b) <= tolerance, "values differ"); }
oh::Organism small() { oh::Organism o; o.p.nx=12; o.p.ny=8; o.init(); return o; }
int main() {
 try {
  auto a=small(); auto b=a;
  for (int k=0;k<200;++k) { a.step_once(); b.step_once(); }
  check(a.hash()==b.hash(), "determinism");
  auto m=a.metrics(); for (double residual:m.resid) close(residual,0,1e-9);
  check(m.min_conc>=0, "negative field");
  check(m.alive+m.dead+a.led.cleared == small().metrics().alive+a.led.births, "population ledger");
  auto d=small(); d.p.k_exchange=0; d.p.seed_fraction=0; d.init();
  for (auto& field:d.c) std::fill(field.begin(),field.end(),0);
  d.c[0][0]=1; d.diffuse(); close(d.c[0][0],.8); close(d.c[0][1],.1); close(d.c[0][12],.1); close(d.mass(0),1);
  for(int k=0;k<200;++k)d.diffuse();close(d.mass(0),1);check(d.metrics().min_conc>=0,"diffusion positivity");
  // No-flux discrete cosine eigenmode: test the spatial operator against
  // its independently known one-step eigenvalue, not another implementation.
  auto mode=small(); const double pi=std::acos(-1.0);
  for(int64_t y=0;y<mode.p.ny;++y) for(int64_t x=0;x<mode.p.nx;++x)
    mode.c[0][y*mode.p.nx+x]=1+0.2*std::cos(pi*(x+0.5)/mode.p.nx);
  mode.diffuse();
  double factor=1-4*mode.p.D[0]*mode.p.dt*std::pow(std::sin(pi/(2*mode.p.nx)),2);
  for(int64_t y=0;y<mode.p.ny;++y) for(int64_t x=0;x<mode.p.nx;++x)
    close(mode.c[0][y*mode.p.nx+x],1+0.2*factor*std::cos(pi*(x+0.5)/mode.p.nx),1e-14);
  auto uniform=small(); auto original=uniform.c; uniform.diffuse(); check(uniform.c==original,"constant field invariance");
  auto death=small();death.p.Vg=0;death.p.Vo=0;death.p.maintenance=10;death.step_once();check(death.metrics().alive==0,"starvation");
  auto division=small();division.p.r_div=10000; for(auto& e:division.energy)e=1;division.fate();check(division.led.births>0,"division");
  auto poison=small(); for(auto& v:poison.c[2])v=10000;poison.step_once();check(poison.led.poisoned>0,"toxicity");
  auto recovery=small();recovery.p.Vg=0;recovery.p.Vo=0;recovery.p.maintenance=0;recovery.damage[0]=.5;recovery.metabolism();close(recovery.damage[0],recovery.state[0]==oh::ALIVE?.499:.5);
  auto e=small(); oh::set_kv(e.p,e.events,"event","1 supply_glucose 0.25");oh::validate_events(e.p,e.events);
  for(int k=0;k<10;++k)e.step_once();close(e.p.supply[0],5);e.step_once();close(e.p.supply[0],.25);
  auto path=std::filesystem::temp_directory_path()/"openhuman-core-test.ckpt";
  e.save(path.string());oh::Organism resumed;resumed.load(path.string());check(e.hash()==resumed.hash(),"checkpoint roundtrip");
  for(int k=0;k<50;++k){e.step_once();resumed.step_once();}check(e.hash()==resumed.hash(),"checkpoint continuation");
  for(int j=0;j<3;++j)close(e.led.consumed[j],resumed.led.consumed[j],0);
  {std::ofstream out(path);out<<"OHCK 2\n";}auto before=resumed.hash();rejects([&]{resumed.load(path.string());});check(before==resumed.hash(),"load must be transactional");std::filesystem::remove(path);
  for (const auto& name:oh::parameter_names()) {auto p=a.p;*oh::dfield(p,name)=std::numeric_limits<double>::quiet_NaN();rejects([&]{oh::validate(p);});}
  for(const auto& text:{"2junk","-1","18446744073709551616"})rejects([&]{oh::number<uint64_t>(text);});
  for(const auto& event:{"-1 Vg 1","1 dt .1","1 nx 5","1 Vg 2 junk","0.15 Vg 1","1 D_oxygen 100","1 maintenance -1"}) {
    auto p=a.p;std::vector<oh::Event> events;rejects([&]{oh::set_kv(p,events,"event",event);oh::validate_events(p,events);});
  }
  a.init();check(a.led.births==0 && a.led.consumed[0]==0,"init resets ledger");
  std::cout<<"All core checks passed\n";
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
